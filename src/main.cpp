#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/file.hpp>
#include <boost/beast/core/file_base.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/serializer.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/vector_body.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/system/detail/error_code.hpp>
#include <fmt/core.h>
#include <iofox.hpp>
#include <twitch.hpp>
#include <util.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace asio = boost::asio;			// NOLINT.
namespace beast = boost::beast;			// NOLINT.
namespace http = beast::http;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto coro() -> io::coro<void>
{
	asio::ssl::stream<asio::ip::tcp::socket> stream {co_await this_coro::executor, co_await io::ssl::context()};
	co_await asio::async_connect(stream.next_layer(), co_await io::dns::resolve("https", "httpbin.org"), io::use_coro);
	io::ssl::set_tls_extension_hostname(stream, "httpbin.org");
	co_await stream.async_handshake(stream.client, io::use_coro);

	// Simple write.
	// io::http::request<std::string> request {"POST", "/post", {{"host", "httpbin.org"}, {"key", "val"}}, "bodydata"};
	// co_await beast::http::async_write(stream, request, io::use_coro);

	// Another write.
	// io::http::request<std::string> request {"POST", "/post", {{"host", "httpbin.org"}, {"key", "val"}}, "bodydata"};
	// beast::http::request_serializer<beast::http::string_body> serializer {request};
	// co_await beast::http::async_write(stream, serializer, io::use_coro);

	// // Write: request_header + body.
	// {
	// 	// User data.
	// 	io::http::request_header request_header {"POST", "/post", {{"host", "httpbin.org"}, {"key", "val"}}};

	// 	// Write request header.
	// 	beast::http::request<beast::http::string_body> request {std::move(request_header)};
	// 	request.set("Content-Length", "8");
	// 	beast::http::request_serializer<beast::http::string_body> serializer {request};
	// 	co_await beast::http::async_write_header(stream, serializer, io::use_coro);

	// 	// Write request body.
	// 	request.body() = "somebody";
	// 	co_await beast::http::async_write(stream, serializer, io::use_coro);
	// }

	// Write: request_header + body (with different body).
	{
		// User data.
		io::http::request_header request_header {"POST", "/post", {{"host", "httpbin.org"}, {"key", "val"}}};

		// Write request header.
		beast::http::request<beast::http::string_body> request {std::move(request_header)};
		request.set("Content-Length", "3");
		beast::http::request_serializer<beast::http::string_body> serializer {request};
		co_await beast::http::async_write_header(stream, serializer, io::use_coro);

		// Write request body.
		beast::http::request<beast::http::vector_body<char>> request_2 {std::move(request)};
		request_2.body() = {'s', 'a', 's'};
		beast::http::request_serializer<beast::http::vector_body<char>> serializer_2 {request_2};
		serializer_2.consume(1);
		co_await beast::http::async_write(stream, serializer_2, io::use_coro);
	}

	beast::flat_buffer buf;
	io::http::response<std::string> response;
	co_await beast::http::async_read(stream, buf, response, io::use_coro);

	std::cout << response << '\n';
}

int main() try
{
	io::windows::set_asio_locale(io::windows::lang::english);
	asio::io_context ctx;
	asio::co_spawn(ctx, coro(), io::rethrowed);
	return ctx.run();
}
catch(std::exception & e) { fmt::print("Exception: '{}'.\n", e.what()); }
catch(...) { fmt::print("Exception: 'unknown'.\n"); }
