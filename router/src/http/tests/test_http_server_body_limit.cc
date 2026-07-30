/*
  Copyright (c) 2026, Oracle and/or its affiliates.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is designed to work with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have either included with
  the program or referenced in the documentation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/**
 * Bug#39268619 - HTTP request body / headers size limits.
 *
 * Starts a real libevent HTTP server (EventHttp), sends requests via
 * RestClient / HttpClient, and verifies oversized bodies are rejected with
 * 413 and oversized headers with 400, without invoking the request handler.
 */

#include <atomic>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>

#include <gtest/gtest.h>

#include "mysql/harness/net_ts/internet.h"
#include "mysql/harness/net_ts/io_context.h"
#include "mysqlrouter/http_client.h"
#include "mysqlrouter/http_common.h"
#include "mysqlrouter/http_request.h"
#include "mysqlrouter/rest_client.h"

namespace {

class TestHttpServer {
 public:
  TestHttpServer(uint64_t max_request_body_size,
                 uint64_t max_request_headers_size)
      : max_request_body_size_{max_request_body_size},
        max_request_headers_size_{max_request_headers_size} {
    net::ip::tcp::endpoint ep(net::ip::address_v4::loopback(), 0);
    auto open_res = listen_sock_.open(ep.protocol());
    if (!open_res) throw std::system_error(open_res.error());

    // evhttp's listener accept-loop calls accept() until EAGAIN. On a
    // blocking listen socket that final accept() blocks inside the event
    // loop and starves already-accepted connections. Match production
    // (HttpRequestMainThread::bind_acceptor).
    auto nb_res = listen_sock_.native_non_blocking(true);
    if (!nb_res) throw std::system_error(nb_res.error());

    auto bind_res = listen_sock_.bind(ep);
    if (!bind_res) throw std::system_error(bind_res.error());
    auto listen_res = listen_sock_.listen(128);
    if (!listen_res) throw std::system_error(listen_res.error());

    auto ep_res = listen_sock_.local_endpoint();
    if (!ep_res) throw std::system_error(ep_res.error());
    port_ = ep_res->port();

    http_.set_allowed_http_methods(HttpMethod::Bitset().set(/* all */));
    http_.set_max_body_size(max_request_body_size_);
    http_.set_max_headers_size(max_request_headers_size_);
    http_.set_gencb(
        [](HttpRequest *req, void *arg) {
          auto *self = static_cast<TestHttpServer *>(arg);
          self->handler_called_.store(true);
          req->send_reply(HttpStatusCode::NoContent);
        },
        this);

    auto bound =
        http_.accept_socket_with_handle(listen_sock_.native_handle());
    if (!bound.is_valid()) {
      throw std::runtime_error("accept_socket_with_handle() failed");
    }

    server_thread_ = std::thread([this] {
      base_.once(
          -1, {EventFlags::Timeout},
          [](EventBase::SocketHandle, short, void *arg) {
            static_cast<TestHttpServer *>(arg)->ready_.store(true);
          },
          this, nullptr);
      base_.dispatch();
    });

    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!ready_.load() && std::chrono::steady_clock::now() < deadline) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (!ready_.load()) {
      throw std::runtime_error("HTTP server event loop failed to start");
    }
  }

  ~TestHttpServer() {
    base_.loop_exit(nullptr);
    if (server_thread_.joinable()) server_thread_.join();
  }

  uint16_t port() const { return port_; }
  bool handler_called() const { return handler_called_.load(); }

 private:
  uint64_t max_request_body_size_;
  uint64_t max_request_headers_size_;
  net::io_context io_ctx_;
  net::ip::tcp::acceptor listen_sock_{io_ctx_};
  uint16_t port_{0};
  EventBase base_;
  EventHttp http_{&base_};
  std::thread server_thread_;
  std::atomic<bool> ready_{false};
  std::atomic<bool> handler_called_{false};
};

class HttpServerSizeLimitTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() { ASSERT_TRUE(Event::initialize_threads()); }

  static void TearDownTestSuite() { Event::shutdown(); }
};

TEST_F(HttpServerSizeLimitTest, OversizedBodyReturns413) {
  // Small limit keeps the test light; production EventHttp construction uses
  // kHttpMaxRequestBodySize via the same set_max_body_size() path.
  constexpr uint64_t kBodyLimit{8};
  TestHttpServer server(kBodyLimit, kHttpMaxRequestHeaderSize);

  IOContext io_ctx;
  RestClient client(io_ctx, "127.0.0.1", server.port());
  auto req =
      client.request_sync(HttpMethod::Post, "/upload", std::string(16, 'a'),
                          "application/octet-stream");

  ASSERT_TRUE(req) << client.error_msg();
  EXPECT_EQ(req.get_response_code(), HttpStatusCode::PayloadTooLarge);
  EXPECT_FALSE(server.handler_called());
}

TEST_F(HttpServerSizeLimitTest, BodyWithinLimitReachesHandler) {
  constexpr uint64_t kBodyLimit{32};
  TestHttpServer server(kBodyLimit, kHttpMaxRequestHeaderSize);

  IOContext io_ctx;
  RestClient client(io_ctx, "127.0.0.1", server.port());
  auto req =
      client.request_sync(HttpMethod::Post, "/upload", std::string(16, 'a'),
                          "application/octet-stream");

  ASSERT_TRUE(req) << client.error_msg();
  EXPECT_EQ(req.get_response_code(), HttpStatusCode::NoContent);
  EXPECT_TRUE(server.handler_called());
}

TEST_F(HttpServerSizeLimitTest, OversizedHeadersReturn400) {
  // Libevent maps oversized headers to EVREQ_HTTP_INVALID_HEADER → 400.
  constexpr uint64_t kHeaderLimit{200};
  TestHttpServer server(kHttpMaxRequestBodySize, kHeaderLimit);

  IOContext io_ctx;
  HttpClient client(io_ctx, "127.0.0.1", server.port());
  HttpRequest req{HttpRequest::sync_callback, nullptr};
  req.get_output_headers().add("Host", "127.0.0.1");
  req.get_output_headers().add("Connection", "close");
  req.get_output_headers().add("X-Big", std::string(300, 'a').c_str());
  client.make_request_sync(&req, HttpMethod::Get, "/");

  ASSERT_TRUE(req) << client.error_msg();
  EXPECT_EQ(req.get_response_code(), HttpStatusCode::BadRequest);
  EXPECT_FALSE(server.handler_called());
}

TEST_F(HttpServerSizeLimitTest, HeadersWithinLimitReachHandler) {
  constexpr uint64_t kHeaderLimit{1024};
  TestHttpServer server(kHttpMaxRequestBodySize, kHeaderLimit);

  IOContext io_ctx;
  HttpClient client(io_ctx, "127.0.0.1", server.port());
  HttpRequest req{HttpRequest::sync_callback, nullptr};
  req.get_output_headers().add("Host", "127.0.0.1");
  req.get_output_headers().add("Connection", "close");
  client.make_request_sync(&req, HttpMethod::Get, "/");

  ASSERT_TRUE(req) << client.error_msg();
  EXPECT_EQ(req.get_response_code(), HttpStatusCode::NoContent);
  EXPECT_TRUE(server.handler_called());
}

}  // namespace

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
