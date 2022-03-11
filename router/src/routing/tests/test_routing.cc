/*
  Copyright (c) 2015, 2021, Oracle and/or its affiliates.

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

#include <array>
#include <stdexcept>
#include <system_error>

#include <gmock/gmock.h>  // EXPECT_THAT
#include <gtest/gtest.h>
#include <gtest/gtest_prod.h>  // FRIEND_TEST

#include "common.h"
#include "mysql/harness/loader.h"
#include "mysql/harness/net_ts/buffer.h"
#include "mysql/harness/net_ts/impl/poll.h"
#include "mysql/harness/net_ts/impl/resolver.h"
#include "mysql/harness/net_ts/impl/socket.h"
#include "mysql/harness/net_ts/impl/socket_constants.h"
#include "mysql/harness/net_ts/impl/socket_error.h"
#include "mysql/harness/net_ts/internet.h"
#include "mysql/harness/net_ts/io_context.h"
#include "mysql/harness/net_ts/local.h"
#include "mysql/harness/net_ts/socket.h"
#include "mysql/harness/stdx/expected.h"
#include "mysql/harness/stdx/expected_ostream.h"
#include "mysql_routing.h"  // AccessMode
#include "mysql_routing_common.h"
#include "mysqlrouter/io_backend.h"
#include "mysqlrouter/io_component.h"
#include "mysqlrouter/routing.h"
#include "tcp_port_pool.h"
#include "test/helpers.h"  // init_test_logger

using mysql_harness::TCPAddress;
using routing::AccessMode;
using namespace std::chrono_literals;

using ::testing::Eq;
using ::testing::Return;
using ::testing::StrEq;

class RoutingTests : public ::testing::Test {
 protected:
  net::io_context io_ctx_;
};

TEST_F(RoutingTests, AccessModes) {
  ASSERT_EQ(static_cast<int>(AccessMode::kReadWrite), 1);
  ASSERT_EQ(static_cast<int>(AccessMode::kReadOnly), 2);
}

TEST_F(RoutingTests, AccessModeLiteralNames) {
  using routing::get_access_mode;
  ASSERT_THAT(get_access_mode("read-write"), Eq(AccessMode::kReadWrite));
  ASSERT_THAT(get_access_mode("read-only"), Eq(AccessMode::kReadOnly));
}

TEST_F(RoutingTests, GetAccessLiteralName) {
  using routing::get_access_mode_name;
  ASSERT_THAT(get_access_mode_name(AccessMode::kReadWrite),
              StrEq("read-write"));
  ASSERT_THAT(get_access_mode_name(AccessMode::kReadOnly), StrEq("read-only"));
}

TEST_F(RoutingTests, Defaults) {
  ASSERT_EQ(routing::kDefaultWaitTimeout, 0);
  ASSERT_EQ(routing::kDefaultMaxConnections, 0);
  ASSERT_EQ(routing::kDefaultDestinationConnectionTimeout,
            std::chrono::seconds(1));
  ASSERT_EQ(routing::kDefaultBindAddress, "127.0.0.1");
  ASSERT_EQ(routing::kDefaultNetBufferLength, 16384U);
  ASSERT_EQ(routing::kDefaultMaxConnectErrors, 100ULL);
  ASSERT_EQ(routing::kDefaultClientConnectTimeout, std::chrono::seconds(9));
}

// a valid Connection::Close xprotocol message
static const std::array<char, 5> kByeMessage = {{0x01, 0x00, 0x00, 0x00, 0x03}};

class MockServer {
 public:
  MockServer() = default;

  ~MockServer() { stop(); }

  stdx::expected<void, std::error_code> start(
      const net::ip::tcp::endpoint &ep) {
    const auto protocol = ep.protocol();

    const auto socket_res = service_tcp_.open(protocol);
    if (!socket_res) {
      return stdx::make_unexpected(socket_res.error());
    }

    net::socket_base::reuse_address reuse_opt{true};
    const auto sockopt_res = service_tcp_.set_option(reuse_opt);
    if (!sockopt_res) {
      return stdx::make_unexpected(sockopt_res.error());
    }

    const auto bind_res = service_tcp_.bind(ep);
    if (!bind_res) {
      return stdx::make_unexpected(bind_res.error());
    }

    const auto listen_res = service_tcp_.listen(20);
    if (!listen_res) {
      return stdx::make_unexpected(listen_res.error());
    }

    stop_ = false;
    thread_ = std::thread(&MockServer::runloop, this);

    return {};
  }

  void stop() {
    if (thread_.joinable()) {
      // signal acceptor thread to exit.
      stop_ = true;
      thread_.join();
    }
  }

  void stop_after_n_accepts(int c) { max_expected_accepts_ = c; }

  void runloop() {
    mysql_harness::rename_thread("Mock::runloop");
    std::vector<std::thread> client_threads;

    while (!stop_ && (max_expected_accepts_ == 0 ||
                      num_accepts_ < max_expected_accepts_)) {
      std::array<struct pollfd, 1> fds = {{
          {service_tcp_.native_handle(), POLLIN, 0},
      }};

      const auto poll_res = net::impl::poll::poll(fds.data(), fds.size(), 10ms);
      if (!poll_res) {
        if (poll_res.error() == make_error_condition(std::errc::interrupted) ||
            poll_res.error() ==
                make_error_condition(std::errc::operation_would_block) ||
            poll_res.error() == make_error_condition(std::errc::timed_out)) {
          // no event yet, restart.
          continue;
        }

        std::cout << __LINE__ << ": mock-server: poll(): " << poll_res.error()
                  << std::endl;

        return;
      }

      auto sock_client_res = service_tcp_.accept();
      if (!sock_client_res) {
        std::cout << "mock-server: accept() "
                  << sock_client_res.error().message() << "\n";
        continue;
      }
      num_accepts_++;

      class Scope {
       public:
        Scope(MockServer *self, net::ip::tcp::socket &&sock_client)
            : self_{self}, sock_client_{std::move(sock_client)} {}

        void operator()() {
          mysql_harness::rename_thread("new_client()");
          self_->num_connections_++;

          do {
            // block until we receive the bye msg
            std::array<struct pollfd, 1> fds = {{
                {sock_client_.native_handle(), POLLIN, 0},
            }};

            const auto poll_res =
                net::impl::poll::poll(fds.data(), fds.size(), 1000ms);
            if (!poll_res) {
              if (poll_res.error() ==
                  make_error_condition(std::errc::interrupted)) {
                continue;
              }

              FAIL() << ": mock-server: poll(): " << poll_res.error() << " "
                     << poll_res.error().message() << std::endl;

              break;
            } else {
              std::array<char, kByeMessage.size()> buf;
              auto read_res = net::impl::socket::read(
                  sock_client_.native_handle(), buf.data(), buf.size());
              if (!read_res) {
                FAIL() << "Unexpected results from read(): "
                       << read_res.error().message();
              }
              break;
            }
          } while (true);

          self_->num_connections_--;
        }

       private:
        MockServer *self_;

        net::ip::tcp::socket sock_client_;
      };

      client_threads.emplace_back(
          Scope(this, std::move(sock_client_res.value())));
    }

    // wait for all threads to shut down again
    for (auto &thr : client_threads) {
      thr.join();
    }
  }

 public:
  std::atomic_int num_connections_{0};
  std::atomic_int num_accepts_{0};
  std::atomic_int max_expected_accepts_{0};

 private:
  std::thread thread_;
  net::io_context io_ctx_;
  net::ip::tcp::acceptor service_tcp_{io_ctx_};
  std::atomic_bool stop_;
};

static stdx::expected<net::ip::tcp::socket, std::error_code> connect_tcp(
    net::io_context &io_ctx, const std::string &host, uint16_t port,
    std::chrono::milliseconds connect_timeout) {
  net::ip::tcp::resolver resolver(io_ctx);

  const auto resolve_res = resolver.resolve(host, std::to_string(port));
  if (!resolve_res) {
    return resolve_res.get_unexpected();
  }

  std::error_code last_ec{make_error_code(std::errc::address_not_available)};

  // try all known addresses of the hostname
  for (const auto &resolved : resolve_res.value()) {
    net::ip::tcp::socket sock(io_ctx);
    const auto open_res = sock.open(resolved.endpoint().protocol());
    if (!open_res) {
      continue;
    }

    sock.native_non_blocking(true);
    const auto connect_res = sock.connect(resolved.endpoint());

    if (!connect_res) {
      if (connect_res.error() ==
              make_error_condition(std::errc::operation_in_progress) ||
          connect_res.error() ==
              make_error_condition(std::errc::operation_would_block)) {
        std::array<pollfd, 1> fds = {{{sock.native_handle(), POLLOUT, 0}}};
        const auto wait_res =
            net::impl::poll::poll(fds.data(), fds.size(), connect_timeout);

        if (!wait_res) {
          last_ec = wait_res.error();
        } else {
          net::socket_base::error sock_err;
          const auto status_res = sock.get_option(sock_err);
          if (!status_res) {
            last_ec = status_res.error();

          } else if (sock_err.value() != 0) {
            last_ec = net::impl::socket::make_error_code(sock_err.value());

          } else {
            // success, we can continue
            sock.native_non_blocking(false);
            return sock;
          }
        }
      } else {
        last_ec = connect_res.error();
      }
    } else {
      // everything is fine, we are connected
      sock.native_non_blocking(false);

      return sock;
    }

    // it failed, try the next address
  }

  return stdx::make_unexpected(last_ec);
}

#ifndef _WIN32  // [_HERE_]

static stdx::expected<net::ip::tcp::socket, std::error_code> connect_local(
    net::io_context &io_ctx, uint16_t port) {
  return connect_tcp(io_ctx, "127.0.0.1", port, 100ms);
}

template <class SyncSocketType>
static void disconnect(SyncSocketType &sock) {
  const auto write_res = net::write(sock, net::buffer(kByeMessage));
  if (!write_res) {
    std::cout << "write(xproto-connection-close) returned error: "
              << write_res.error().message() << "\n";
  }

  sock.shutdown(net::socket_base::shutdown_both);

  // wait until the shutdown is acknowledged.
  std::array<uint8_t, 16> read_buf;
  const auto read_res = net::read(sock, net::buffer(read_buf));
  if (!read_res && read_res.error() != make_error_code(net::stream_errc::eof)) {
    std::cout << "read::linger(xproto-connection-close) returned error: "
              << read_res.error().message() << "\n";
  }

  sock.close();
}

#ifndef _WIN32
static stdx::expected<local::stream_protocol::socket, std::error_code>
connect_socket(net::io_context &io_ctx,
               const local::stream_protocol::endpoint &ep) {
  local::stream_protocol::socket sock(io_ctx);
  const auto socket_res = sock.open();
  if (!socket_res) {
    return stdx::make_unexpected(socket_res.error());
  }

  const auto connect_res = sock.connect(ep);
  if (!connect_res) {
    return stdx::make_unexpected(socket_res.error());
  }

  return sock;
}
#endif

static bool call_until(std::function<bool()> f, int timeout = 2) {
  time_t start = time(nullptr);
  while (time(nullptr) - start < timeout) {
    if (f()) return true;

    // wait a bit and let other threads run
    std::this_thread::sleep_for(10ms);
  }
  return false;
}

// Bug#24841281 NOT ABLE TO CONNECT ANY CLIENTS WHEN ROUTER IS CONFIGURED WITH
// SOCKETS OPTION
TEST_F(RoutingTests, bug_24841281) {
  mysql_harness::rename_thread("TEST_F()");

  TcpPortPool port_pool_;

  const uint16_t server_port = port_pool_.get_next_available();
  const uint16_t router_port = port_pool_.get_next_available();
  using socket_res_type = stdx::expected<net::ip::tcp::socket, std::error_code>;

  const net::ip::tcp::endpoint server_endpoint(net::ip::tcp::v4(), server_port);

  MockServer server;
  ASSERT_THAT(server.start(server_endpoint),
              ::testing::Truly([](const auto &v) { return bool(v); }))
      << server_endpoint;

  TmpDir tmp_dir;  // create a tmp dir (it will be destroyed via RAII later)
  mysql_harness::Path sock_path
#ifndef _WIN32
      (tmp_dir() + "/sock")
#endif
          ;

  const int expected_accepts{
#ifdef _WIN32
      4
#else
      6
#endif
  };

  class Ctx {
   public:
    Ctx() : io_comp_{IoComponent::get_instance()} {
      // init the IoComponent
      io_comp_.init(1, IoBackend::preferred());
      guards_.emplace_back(io_comp_);
      io_thd_ = std::thread([&]() {
        io_comp_.run();
        std::cerr << "test: io-context finished" << std::endl;
      });
    }

    net::io_context &io_context() { return io_comp_.io_context(); }

    ~Ctx() {
      // release the Workguard to allow the io_comp_.run() to stop.
      guards_.clear();
      io_thd_.join();

      io_comp_.reset();
    }

   private:
    IoComponent &io_comp_;
    // create some workguards to be able to start the iocontext (and keep it
    // running) in one thread while the routing thread starts.
    std::list<IoComponent::Workguard> guards_;

    std::thread io_thd_;
  };

  Ctx ctx;

  net::io_context &io_ctx = ctx.io_context();

  // check that connecting to a TCP socket or a UNIX socket works
  MySQLRouting routing(
      io_ctx, routing::RoutingStrategy::kNextAvailable, router_port,
      Protocol::Type::kXProtocol, routing::AccessMode::kReadWrite, "0.0.0.0",
      sock_path, "routing:testroute", routing::kDefaultMaxConnections,
      routing::kDefaultDestinationConnectionTimeout,
      routing::kDefaultMaxConnectErrors, routing::kDefaultClientConnectTimeout,
      routing::kDefaultNetBufferLength);
  routing.set_destinations_from_csv("127.0.0.1:" + std::to_string(server_port));
  mysql_harness::ConfigSection cs{"routing", "testroute", nullptr};
  mysql_harness::PluginFuncEnv env(nullptr, &cs, true);

  std::thread thd(&MySQLRouting::start, &routing, &env);

  mysql_harness::ScopeGuard guard([&]() {
    env.clear_running();  // shut down MySQLRouting
    routing.stop_socket_acceptors();
    server.stop();
    thd.join();
  });

  mysql_harness::Config conf;
  MySQLRoutingComponent::get_instance().init(conf);

  // set the number of accepts that the server should expect for before
  // stopping
  server.stop_after_n_accepts(expected_accepts);

  EXPECT_EQ(routing.get_context().info_active_routes_.load(), 0);

  // open connections to the socket and see if we get a matching outgoing
  // socket connection attempt to our mock server

  socket_res_type sock1_res{
      stdx::make_unexpected(make_error_code(std::errc::not_a_socket))};
  // router is running in a thread, so we need to sync it
  EXPECT_TRUE(call_until([&]() -> bool {
    sock1_res = connect_local(io_ctx, router_port);
    return sock1_res.has_value();
  })) << "timed out connecting to router_port";
  auto sock2_res = connect_local(io_ctx, router_port);

  ASSERT_TRUE(sock1_res);
  ASSERT_TRUE(sock2_res);

  auto sock1 = std::move(sock1_res.value());
  auto sock2 = std::move(sock2_res.value());

  EXPECT_TRUE(call_until([&server]() -> bool {
    return server.num_connections_.load() == 2;
  })) << "timed out, got "
      << server.num_connections_.load() << " connections";
  EXPECT_TRUE(call_until([&routing]() -> bool {
    return routing.get_context().info_active_routes_.load() == 2;
  })) << "timed out, got "
      << routing.get_context().info_active_routes_.load() << " active routes";

  disconnect(sock1);

  EXPECT_TRUE(call_until([&routing]() -> bool {
    return routing.get_context().info_active_routes_.load() == 1;
  })) << "timed out, got "
      << routing.get_context().info_active_routes_.load() << " active routes";

  {
    auto sock11_res = connect_local(io_ctx, router_port);
    auto sock12_res = connect_local(io_ctx, router_port);

    ASSERT_TRUE(sock11_res);
    ASSERT_TRUE(sock12_res);

    auto sock11 = std::move(sock11_res.value());
    auto sock12 = std::move(sock12_res.value());

    EXPECT_TRUE(call_until([&server]() -> bool {
      return server.num_connections_.load() == 3;
    })) << "timed out: "
        << server.num_connections_.load();

    call_until([&routing]() -> bool {
      return routing.get_context().info_active_routes_.load() == 3;
    });
    EXPECT_EQ(3, routing.get_context().info_active_routes_.load());

    disconnect(sock11);
    call_until([&routing]() -> bool {
      return routing.get_context().info_active_routes_.load() == 2;
    });
    EXPECT_EQ(2, routing.get_context().info_active_routes_.load());

    disconnect(sock12);
    call_until([&routing]() -> bool {
      return routing.get_context().info_active_routes_.load() == 1;
    });
    EXPECT_EQ(1, routing.get_context().info_active_routes_.load());

    call_until(
        [&server]() -> bool { return server.num_connections_.load() == 1; });
    EXPECT_EQ(1, server.num_connections_.load());
  }

  disconnect(sock2);
  call_until([&routing]() -> bool {
    return routing.get_context().info_active_routes_.load() == 0;
  });
  EXPECT_EQ(0, routing.get_context().info_active_routes_.load());

#ifndef _WIN32
  SCOPED_TRACE("// open/close two unix-socket connections and check counters");
  // now try the same with unix sockets
  const auto unix_sock_ep = local::stream_protocol::endpoint(sock_path.str());
  auto sock3_res = connect_socket(io_ctx, unix_sock_ep);
  auto sock4_res = connect_socket(io_ctx, unix_sock_ep);

  ASSERT_TRUE(sock3_res);
  ASSERT_TRUE(sock4_res);

  auto sock3 = std::move(sock3_res.value());
  auto sock4 = std::move(sock4_res.value());

  call_until(
      [&server]() -> bool { return server.num_connections_.load() == 2; });
  EXPECT_EQ(2, server.num_connections_.load());

  call_until([&routing]() -> bool {
    return routing.get_context().info_active_routes_.load() == 2;
  });
  EXPECT_EQ(2, routing.get_context().info_active_routes_.load());

  disconnect(sock3);
  call_until([&routing]() -> bool {
    return routing.get_context().info_active_routes_.load() == 1;
  });
  EXPECT_EQ(1, routing.get_context().info_active_routes_.load());

  SCOPED_TRACE(
      "// close the last connect and check the active routes decrease.");
  disconnect(sock4);
  call_until([&routing]() -> bool {
    return routing.get_context().info_active_routes_.load() == 0;
  });
  EXPECT_EQ(0, routing.get_context().info_active_routes_.load());
#endif
}
#endif  // #ifndef _WIN32 [_HERE_]

TEST_F(RoutingTests, set_destinations_from_uri) {
  MySQLRouting routing(io_ctx_, routing::RoutingStrategy::kFirstAvailable, 7001,
                       Protocol::Type::kXProtocol);

  // valid metadata-cache uri
  {
    URI uri("metadata-cache://test/default?role=PRIMARY");
    EXPECT_NO_THROW(routing.set_destinations_from_uri(uri));
  }

  // metadata-cache uri, role missing
  {
    URI uri("metadata-cache://test/default");
    try {
      routing.set_destinations_from_uri(uri);
      FAIL() << "Expected std::runtime_error exception";
    } catch (const std::runtime_error &err) {
      EXPECT_EQ(
          err.what(),
          std::string("Missing 'role' in routing destination specification"));
    } catch (...) {
      FAIL() << "Expected std::runtime_error exception";
    }
  }

  // invalid scheme
  {
    URI uri("invalid-scheme://test/default?role=SECONDARY");
    try {
      routing.set_destinations_from_uri(uri);
      FAIL() << "Expected std::runtime_error exception";
    } catch (const std::runtime_error &err) {
      EXPECT_EQ(err.what(),
                std::string("Invalid URI scheme; expecting: 'metadata-cache' "
                            "is: 'invalid-scheme'"));
    } catch (...) {
      FAIL() << "Expected std::runtime_error exception";
    }
  }
}

TEST_F(RoutingTests, set_destinations_from_cvs) {
  MySQLRouting routing(io_ctx_, routing::RoutingStrategy::kNextAvailable, 7001,
                       Protocol::Type::kXProtocol);

  // valid address list
  {
    const std::string cvs = "127.0.0.1:2002,127.0.0.1:2004";
    EXPECT_NO_THROW(routing.set_destinations_from_csv(cvs));
  }

  // no routing strategy, should go with default
  {
    MySQLRouting routing_inv(io_ctx_, routing::RoutingStrategy::kUndefined,
                             7001, Protocol::Type::kXProtocol);
    const std::string csv = "127.0.0.1:2002,127.0.0.1:2004";
    EXPECT_NO_THROW(routing_inv.set_destinations_from_csv(csv));
  }

  // no address
  {
    const std::string csv = "";
    EXPECT_THROW(routing.set_destinations_from_csv(csv), std::runtime_error);
  }

  // invalid address
  {
    const std::string csv = "127.0.0..2:2222";
    EXPECT_THROW(routing.set_destinations_from_csv(csv), std::runtime_error);
  }

  // let's check if the correct defualt port gets chosen for
  // the respective protocol
  // we use the trick here setting the expected address also as
  // the binding address for the routing which should make the method throw
  // an exception if these are the same
  {
    const std::string address = "127.0.0.1";
    MySQLRouting routing_classic(io_ctx_,
                                 routing::RoutingStrategy::kNextAvailable, 3306,
                                 Protocol::Type::kClassicProtocol,
                                 routing::AccessMode::kReadWrite, address);
    EXPECT_THROW(routing_classic.set_destinations_from_csv("127.0.0.1"),
                 std::runtime_error);
    EXPECT_THROW(routing_classic.set_destinations_from_csv("127.0.0.1:3306"),
                 std::runtime_error);
    EXPECT_NO_THROW(
        routing_classic.set_destinations_from_csv("127.0.0.1:33060"));

    MySQLRouting routing_x(io_ctx_, routing::RoutingStrategy::kNextAvailable,
                           33060, Protocol::Type::kXProtocol,
                           routing::AccessMode::kReadWrite, address);
    EXPECT_THROW(routing_x.set_destinations_from_csv("127.0.0.1"),
                 std::runtime_error);
    EXPECT_THROW(routing_x.set_destinations_from_csv("127.0.0.1:33060"),
                 std::runtime_error);
    EXPECT_NO_THROW(routing_x.set_destinations_from_csv("127.0.0.1:3306"));
  }
}

TEST_F(RoutingTests, get_routing_thread_name) {
  // config name must begin with "routing" (name of the plugin passed from
  // configuration file)
  EXPECT_STREQ(":parse err", get_routing_thread_name("", "").c_str());
  EXPECT_STREQ(":parse err", get_routing_thread_name("routin", "").c_str());
  EXPECT_STREQ(":parse err", get_routing_thread_name(" routing", "").c_str());
  EXPECT_STREQ("pre:parse err", get_routing_thread_name("", "pre").c_str());
  EXPECT_STREQ("pre:parse err",
               get_routing_thread_name("routin", "pre").c_str());
  EXPECT_STREQ("pre:parse err",
               get_routing_thread_name(" routing", "pre").c_str());

  // normally prefix would never be empty, so the behavior below is not be very
  // meaningful; it should not crash however
  EXPECT_STREQ(":", get_routing_thread_name("routing", "").c_str());
  EXPECT_STREQ(":", get_routing_thread_name("routing:", "").c_str());

  // realistic (but unanticipated) cases - removing everything up to _default_
  // will fail, in which case we fall back of <prefix>:<everything after
  // "routing:">, trimmed to 15 chars
  EXPECT_STREQ(
      "RtS:test_def_ul",
      get_routing_thread_name("routing:test_def_ult_x_ro", "RtS").c_str());
  EXPECT_STREQ(
      "RtS:test_def_ul",
      get_routing_thread_name("routing:test_def_ult_ro", "RtS").c_str());
  EXPECT_STREQ("RtS:", get_routing_thread_name("routing", "RtS").c_str());
  EXPECT_STREQ("RtS:test_x_ro",
               get_routing_thread_name("routing:test_x_ro", "RtS").c_str());
  EXPECT_STREQ("RtS:test_ro",
               get_routing_thread_name("routing:test_ro", "RtS").c_str());

  // real cases
  EXPECT_STREQ(
      "RtS:x_ro",
      get_routing_thread_name("routing:test_default_x_ro", "RtS").c_str());
  EXPECT_STREQ(
      "RtS:ro",
      get_routing_thread_name("routing:test_default_ro", "RtS").c_str());
  EXPECT_STREQ("RtS:", get_routing_thread_name("routing", "RtS").c_str());
}

/*
 * @test This test verifies fix for Bug 23857183 and checks if trying to connect
 * to wrong port fails immediately not via timeout
 *
 * @todo (jan) disabled the test as the result is unpredictable as port may be
 * in use, IP may be or not be bound, ... The test needs to be rewritten and
 * have predictable output, or be removed.
 */
TEST_F(RoutingTests, DISABLED_ConnectToServerWrongPort) {
  const std::chrono::seconds TIMEOUT{4};
  net::io_context io_ctx;

  // wrong port number
  {
    auto server_res = connect_tcp(io_ctx, "127.0.0.1", 10888, TIMEOUT);
    // should return -1, -2 is timeout expired which is not what we expect when
    // connecting with the wrong port
    ASSERT_FALSE(server_res);
  }

// in darwin and solaris, attempting connection to 127.0.0.11 will fail by
// timeout
#if !defined(__APPLE__) && !defined(__sun)
  // wrong port number and IP
  {
    auto server_res = connect_tcp(io_ctx, "127.0.0.11", 10888, TIMEOUT);
    // should return -1, -2 is timeout expired which is not what we expect when
    // connecting with the wrong port
    ASSERT_FALSE(server_res);
  }
#endif
}

int main(int argc, char *argv[]) {
  net::impl::socket::init();
#ifndef _WIN32
  signal(SIGPIPE, SIG_IGN);
#endif

  init_test_logger();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
