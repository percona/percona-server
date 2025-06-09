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

#include "jit_executor_context_pool.h"
#include "jit_executor_common_context.h"
#include "jit_executor_javascript_context.h"

#include <memory>
#include <string>
#include <vector>

#include "include/my_thread.h"
#include "jit_executor_common_context.h"
#include "jit_executor_javascript_context.h"

namespace jit_executor {

ContextPool::ContextPool(size_t size, CommonContext *common_context)
    : m_common_context{common_context} {
  m_pool = std::make_unique<Pool<IContext *>>(
      size,
      [this](size_t id) -> JavaScriptContext * {
        auto context =
            std::make_unique<JavaScriptContext>(id, m_common_context);
        if (!context->started()) {
          // The factory function should throw runtime exception if fails
          throw std::runtime_error("Failed initializing JavaScriptContext");
        }

        return context.release();
      },
      [](IContext *ctx) { delete ctx; });
}

ContextPool::~ContextPool() { teardown(); }

void ContextPool::teardown() {
  m_pool->teardown();

  // Tear down the releaser thread
  release(nullptr);

  if (m_release_thread) {
    m_release_thread->join();
    m_release_thread.reset();
  }
}

std::shared_ptr<PooledContextHandle> ContextPool::get_context() {
  auto ctx = m_pool->get();

  if (ctx) {
    if (!m_release_thread) {
      m_release_thread =
          std::make_unique<std::thread>(&ContextPool::release_thread, this);
    }

    return std::make_shared<PooledContextHandle>(this, ctx);
  }

  return {};
}

void ContextPool::release(IContext *ctx) { m_release_queue.push(ctx); }

void ContextPool::release_thread() {
  my_thread_self_setname("Jit-CtxDispose");
  while (true) {
    auto ctx = m_release_queue.pop();
    if (ctx) {
      if (ctx->wait_for_idle()) {
        m_pool->release(ctx);
      } else {
        m_pool->discard(ctx, true);
      }
    } else {
      // nullptr arrived, meaning we are done
      break;
    }
  }
}

}  // namespace jit_executor