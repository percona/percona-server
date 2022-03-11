/*
  Copyright (c) 2017, 2021, Oracle and/or its affiliates.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "dest_round_robin.h"

#include <chrono>
#include <iterator>
#include <memory>
#include <stdexcept>

#include "common.h"  // rename_thread
#include "mysql/harness/logging/logging.h"
#include "mysql/harness/net_ts/impl/poll.h"
#include "mysql/harness/net_ts/impl/socket.h"
#include "mysql/harness/net_ts/impl/socket_error.h"
#include "mysql/harness/net_ts/internet.h"
#include "mysql/harness/net_ts/io_context.h"
#include "mysql/harness/net_ts/socket.h"
#include "mysql/harness/stdx/expected.h"
#include "mysqlrouter/destination.h"

IMPORT_LOG_FUNCTIONS()

using mysql_harness::TCPAddress;

using namespace std::chrono_literals;

// Timeout for trying to connect with quarantined servers
static constexpr auto kQuarantinedConnectTimeout = 1000ms;
// How long we pause before checking quarantined servers again (seconds)
static const auto kQuarantineCleanupInterval = 3s;
// Make sure Quarantine Manager Thread is run even with nothing in quarantine
static const auto kTimeoutQuarantineConditional = 2s;

void *DestRoundRobin::run_thread(void *context) {
  static_cast<DestRoundRobin *>(context)->quarantine_manager_thread();
  return nullptr;
}

void DestRoundRobin::start(const mysql_harness::PluginFuncEnv * /*env*/) {
  quarantine_thread_.run(&run_thread, this);
}

class QuanrantinableDestination : public Destination {
 public:
  QuanrantinableDestination(std::string id, std::string host, uint16_t port,
                            DestRoundRobin *balancer, size_t ndx)
      : Destination(std::move(id), std::move(host), port),
        balancer_{balancer},
        ndx_{ndx} {}

  void connect_status(std::error_code ec) override {
    if (ec != std::error_code()) {
      balancer_->add_to_quarantine(ndx_);
    }
  }

  bool good() const override { return !balancer_->is_quarantined(ndx_); }

 private:
  DestRoundRobin *balancer_;
  size_t ndx_;
};

Destinations DestRoundRobin::destinations() {
  Destinations dests;

  {
    std::lock_guard<std::mutex> lk(mutex_update_);

    const auto end = destinations_.end();
    const auto begin = destinations_.begin();
    const auto sz = destinations_.size();
    auto cur = begin;

    // move iterator forward and remember the position as 'last'
    std::advance(cur, start_pos_);
    auto last = cur;
    size_t n = start_pos_;

    // for start_pos == 2:
    //
    // 0 1 2 3 4 x
    // ^   ^     ^
    // |   |     `- end
    // |   `- last|cur
    // `- begin

    // from last to end;
    //
    // dests = [2 3 4]

    for (; cur != end; ++cur, ++n) {
      auto const &dest = *cur;

      dests.push_back(std::make_unique<QuanrantinableDestination>(
          dest.str(), dest.address(), dest.port(), this, n));
    }

    // from begin to before-last
    //
    // dests = [2 3 4] + [0 1]
    //
    for (cur = begin, n = 0; cur != last; ++cur, ++n) {
      auto const &dest = *cur;

      dests.push_back(std::make_unique<QuanrantinableDestination>(
          dest.str(), dest.address(), dest.port(), this, n));
    }

    if (++start_pos_ >= sz) start_pos_ = 0;
  }

  return dests;
}

DestRoundRobin::~DestRoundRobin() {
  stopper_.set_value();

  quarantine_.serialize_with_cv([](auto &, auto &cv) { cv.notify_one(); });
  quarantine_thread_.join();
}

void DestRoundRobin::add_to_quarantine(const size_t index) noexcept {
  assert(index < size());
  if (index >= size()) {
    log_debug("Impossible server being quarantined (index %zu)", index);
    return;
  }

  quarantine_.serialize_with_cv([this, index](auto &q, auto &cv) {
    if (!q.has(index)) {
      log_debug("Quarantine destination server %s (index %zu)",
                destinations_.at(index).str().c_str(), index);

      q.add(index);
      if (q.size() >= this->destinations().size()) {
        this->stop_listening_router_socket();
      }
      cv.notify_one();
    }
  });
}

static stdx::expected<void, std::error_code> tcp_port_alive(
    net::io_context &io_ctx, const std::string &host, uint16_t port,
    std::chrono::milliseconds connect_timeout) {
  net::ip::tcp::resolver resolver(io_ctx);

  const auto resolve_res = resolver.resolve(host, std::to_string(port));
  if (!resolve_res) {
    return resolve_res.get_unexpected();
  }

  std::error_code last_ec{};

  // try all known addresses of the hostname
  for (auto const &resolved : resolve_res.value()) {
    net::ip::tcp::socket sock(io_ctx);

    auto open_res = sock.open(resolved.endpoint().protocol());
    if (!open_res) {
      return open_res.get_unexpected();
    }

    sock.native_non_blocking(true);
    const auto connect_res = sock.connect(resolved.endpoint());

    if (!connect_res) {
      if (connect_res.error() ==
              make_error_condition(std::errc::operation_in_progress) ||
          connect_res.error() ==
              make_error_condition(std::errc::operation_would_block)) {
        std::array<pollfd, 1> pollfds = {{
            {sock.native_handle(), POLLOUT, 0},
        }};

        const auto wait_res = net::impl::poll::poll(
            pollfds.data(), pollfds.size(), connect_timeout);

        if (!wait_res) {
          last_ec = wait_res.error();
        } else {
          net::socket_base::error err;
          const auto status_res = sock.get_option(err);
          if (!status_res) {
            last_ec = status_res.error();
          } else if (err.value() != 0) {
            last_ec = net::impl::socket::make_error_code(err.value());
          } else {
            // success, we can continue
            return {};
          }
        }
      } else {
        last_ec = connect_res.error();
      }
    } else {
      // everything is fine, we are connected
      return {};
    }

    // it failed, try the next address
  }

  return stdx::make_unexpected(last_ec);
}

std::vector<size_t> Quarantine::quarantined() const { return quarantined_; }

void Quarantine::add(size_t ndx) { quarantined_.push_back(ndx); }

bool Quarantine::has(size_t ndx) const {
  const auto it = std::find(quarantined_.begin(), quarantined_.end(), ndx);

  return it != quarantined_.end();
}

size_t Quarantine::size() const { return quarantined_.size(); }

bool Quarantine::empty() const { return quarantined_.empty(); }

void Quarantine::erase(size_t ndx) {
  quarantined_.erase(std::remove(quarantined_.begin(), quarantined_.end(), ndx),
                     quarantined_.end());
}

void DestRoundRobin::cleanup_quarantine() noexcept {
  auto cpy_quarantined(
      quarantine_([](auto const &q) { return q.quarantined(); }));

  // Nothing to do when nothing quarantined
  if (cpy_quarantined.empty()) return;

  for (auto const ndx : cpy_quarantined) {
    if (stopped_.wait_for(0s) == std::future_status::ready) {
      return;
    }

    const auto addr = destinations_.at(ndx);
    const auto sock_res = tcp_port_alive(io_ctx_, addr.address(), addr.port(),
                                         kQuarantinedConnectTimeout);

    if (sock_res) {
      log_debug("Unquarantine destination server %s (index %zu)",
                addr.str().c_str(), ndx);
      quarantine_([ndx](auto &q) { q.erase(ndx); });
    }
  }
}

void DestRoundRobin::quarantine_manager_thread() noexcept {
  mysql_harness::rename_thread(
      "RtQ:<unknown>");  // TODO change <unknown> to instance name

  while (stopped_.wait_for(0s) != std::future_status::ready) {
    stdx::expected<void, std::error_code> start_socket_acceptor_res{};
    // wait until something got added to quarantie or shutdown, do not wait
    // if we could not start socket acceptor in previous iteration - in that
    // case try to start acceptor again
    quarantine_.wait_for(kTimeoutQuarantineConditional,
                         [this, &start_socket_acceptor_res](auto &q) {
                           return !q.empty() || !start_socket_acceptor_res ||
                                  stopped_.wait_for(0s) ==
                                      std::future_status::ready;
                         });

    // if we aren't shutting down, cleanup and wait
    if (stopped_.wait_for(0s) != std::future_status::ready) {
      cleanup_quarantine();
      // If there are unquarantined destinations, listening router socket should
      // be open.
      quarantine_([this, &start_socket_acceptor_res](auto &q) {
        if (q.size() < this->destinations().size()) {
          std::lock_guard<std::mutex> lock(
              socket_acceptor_handle_callbacks_mtx);
          if (this->start_router_socket_acceptor_callback_) {
            start_socket_acceptor_res =
                this->start_router_socket_acceptor_callback_();
          }
        }
      });
      // Temporize
      stopped_.wait_for(kQuarantineCleanupInterval);
    }
  }
}

size_t DestRoundRobin::size_quarantine() {
  return quarantine_([](auto &q) { return q.size(); });
}
