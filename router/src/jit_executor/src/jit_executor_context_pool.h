/*
 * Copyright (c) 2024, 2025, Oracle and/or its affiliates.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0,
 * as published by the Free Software Foundation.
 *
 * This program is designed to work with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms,
 * as designated in a particular file or component or in included license
 * documentation.  The authors of MySQL hereby grant you an additional
 * permission to link the program and your derivative works with the
 * separately licensed software that they have either included with
 * the program or referenced in the documentation.
 *
 * This program is distributed in the hope that it will be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef ROUTER_SRC_JIT_EXECUTOR_INCLUDE_MYSQLROUTER_JIT_EXECUTOR_CONTEXT_POOL_H_
#define ROUTER_SRC_JIT_EXECUTOR_INCLUDE_MYSQLROUTER_JIT_EXECUTOR_CONTEXT_POOL_H_

#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "mysql/harness/logging/logging.h"
#include "mysql/harness/mpsc_queue.h"
#include "mysqlrouter/jit_executor_context.h"
#include "mysqlrouter/jit_executor_context_handle.h"
#include "mysqlrouter/jit_executor_value.h"
#include "mysqlrouter/polyglot_file_system.h"

namespace jit_executor {

IMPORT_LOG_FUNCTIONS()

/**
 * Generic implementation of a pool
 */
template <class T>
class Pool {
 public:
  explicit Pool(size_t size, const std::function<T(size_t)> &factory,
                const std::function<void(T)> &destructor = {})
      : m_pool_size{size},
        m_item_factory{factory},
        m_item_destructor{destructor} {}

  T get() {
    {
      std::unique_lock lock(m_mutex);

      if (m_teardown) return {};

      T item = {};

      // Contention mode is turned ON when a memory error happened, for that
      // reason creating new contexts is a no-go so we should wait for a context
      // from the pool to get released
      if (m_contention_mode && m_items.empty()) {
        m_item_availability.wait(
            lock, [this]() { return m_active_items == 0 || !m_items.empty(); });

        if (m_active_items == 0) {
          throw std::runtime_error(
              "All the contexts on the pool have been released.");
        }
      }

      if (!m_items.empty()) {
        // Pop a resource from the pool
        item = m_items.front();
        m_items.pop_front();
        return item;
      }
    }

    try {
      T item = m_item_factory(m_created_items);
      increase_active_items();
      return item;
    } catch (const std::runtime_error &) {
      // An initialization failure would raise this exception, no action is
      // needed
      return {};
    }
  }

  void release(T ctx) {
    {
      std::scoped_lock lock(m_mutex);

      if (!m_teardown && m_items.size() < m_pool_size) {
        m_items.push_back(ctx);
        m_item_availability.notify_one();
        return;
      }
    }

    discard(ctx, false);
  }

  void teardown() {
    {
      std::scoped_lock lock(m_mutex);
      m_teardown = true;
    }

    while (!m_items.empty()) {
      auto item = m_items.front();
      m_items.pop_front();

      discard(item, false);
    }

    // Waits until all the contexts created by the pool get released
    std::unique_lock<std::mutex> lock(m_mutex);
    m_item_availability.wait(lock, [this]() { return m_active_items == 0; });
  }

  size_t active_items() const {
    std::scoped_lock lock(m_mutex);
    return m_active_items;
  }

  /**
   * Discards the affected context and turns ON contention mode for the pool
   */
  void discard(T ctx, bool set_contention_mode) {
    decrease_active_items(set_contention_mode);

    if (m_item_destructor) {
      try {
        m_item_destructor(ctx);
      } catch (const std::exception &e) {
        log_error("%s", e.what());
      }
    }
  }

 private:
  void increase_active_items() {
    {
      std::scoped_lock lock(m_mutex);
      m_active_items++;
      m_created_items++;
    }
  }

  void decrease_active_items(bool set_contention_mode = false) {
    {
      std::scoped_lock lock(m_mutex);
      m_active_items--;

      if (set_contention_mode) {
        m_contention_mode = true;
      }
    }
    m_item_availability.notify_all();
  }

  std::mutex m_mutex;
  std::condition_variable m_item_availability;
  bool m_teardown = false;
  size_t m_pool_size;
  std::deque<T> m_items;
  std::function<T(size_t id)> m_item_factory;
  std::function<void(T)> m_item_destructor;
  size_t m_active_items = 0;
  size_t m_created_items = 0;
  bool m_contention_mode = false;
};

class PooledContextHandle;
class CommonContext;
class ContextPool final {
 public:
  ContextPool(size_t size, CommonContext *common_context);

  ~ContextPool();

  std::shared_ptr<PooledContextHandle> get_context();
  void release(IContext *ctx);
  void teardown();

 private:
  void release_thread();

  CommonContext *m_common_context;
  std::unique_ptr<Pool<IContext *>> m_pool;
  mysql_harness::WaitingMPSCQueue<IContext *> m_release_queue;
  std::unique_ptr<std::thread> m_release_thread;
};

/**
 * A wrapper that will return a context to the pool as soon as it is released
 */
class PooledContextHandle : public IContextHandle {
 public:
  PooledContextHandle(ContextPool *pool, IContext *ctx)
      : m_pool{pool}, m_context{ctx} {}
  ~PooledContextHandle() override { m_pool->release(m_context); }

  IContext *get() override { return m_context; }

 private:
  ContextPool *m_pool;
  IContext *m_context;
};

}  // namespace jit_executor

#endif  // ROUTER_SRC_JIT_EXECUTOR_INCLUDE_MYSQLROUTER_JIT_EXECUTOR_CONTEXT_POOL_H_