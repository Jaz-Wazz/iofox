#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/impl/file_body_win32.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/serializer.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/write.hpp>
#include <chrono>
#include <fmt/core.h>
#include <iostream>
#include <httpfox.hpp>
#include <stdexcept>
#include <string>
#include <utility>
#include <expected>

namespace asio = boost::asio;					// NOLINT
namespace beast = boost::beast;					// NOLINT
namespace this_coro = boost::asio::this_coro;	// NOLINT

auto perform_http(auto host, auto & request, auto & response) -> asio::awaitable<void>
{
	auto ctx = co_await this_coro::executor;
	asio::ip::tcp::resolver resolver {ctx};
	auto resolver_results = co_await resolver.async_resolve(host, "http", asio::use_awaitable);
	asio::ip::tcp::socket sock {ctx};
	co_await asio::async_connect(sock, resolver_results, asio::use_awaitable);
	request.set("Host", host);
	co_await beast::http::async_write(sock, request, asio::use_awaitable);
	beast::flat_buffer buffer;
	co_await beast::http::async_read(sock, buffer, response, asio::use_awaitable);
};

auto take_archive_location(std::string path) -> asio::awaitable<std::expected<std::pair<std::string, std::string>, int>>
{
	beast::http::request<beast::http::empty_body> request {beast::http::verb::get, "/download" + path, 11};
	beast::http::response<beast::http::string_body> response;
	co_await perform_http("archive.org", request, response);
	if(response.result_int() == 302)
	{
		auto location = response.at("Location");
		co_return std::make_pair(location.substr(7, location.find('/', 7) - 7), location.substr(location.find('/', 7)));
	}
	else co_return std::unexpected(response.result_int());
}

auto http_get_contents(std::string host, std::string path) -> asio::awaitable<std::expected<std::string, int>>
{
	beast::http::request<beast::http::empty_body> request {beast::http::verb::get, path, 11};
	beast::http::response<beast::http::string_body> response;
	co_await perform_http(host, request, response);
	if(response.result_int() == 200) co_return response.body(); else co_return std::unexpected(response.result_int());
}

auto session(beast::tcp_stream stream_user) -> asio::awaitable<void>
{
	// Data.
	auto host = "ia902302.us.archive.org";
	auto path = "/22/items/1_20211016_20211016/1.png";

	// Connect to archive.
	auto resolver_results = co_await asio::ip::tcp::resolver(co_await this_coro::executor).async_resolve(host, "http", asio::use_awaitable);
	asio::ip::tcp::socket sock_archive {co_await this_coro::executor};
	co_await asio::async_connect(sock_archive, resolver_results, asio::use_awaitable);

	// Send request to archive.
	beast::http::request<beast::http::empty_body> request_archive {beast::http::verb::get, path, 11};
	request_archive.set("Host", host);
	co_await beast::http::async_write(sock_archive, request_archive, asio::use_awaitable);

	// Read response header from archive.
	beast::http::response_parser<beast::http::string_body> parser_archive_response;
	beast::flat_buffer buf;
	co_await beast::http::async_read_header(sock_archive, buf, parser_archive_response, asio::use_awaitable);
	std::cout << parser_archive_response.get() << '\n';

	// Prepare user response data.
	beast::http::response<beast::http::buffer_body> response_user;
	beast::http::response_serializer<beast::http::buffer_body> serializer_user {response_user};

	// Incremental read response body from archive.
	while(!parser_archive_response.is_done())
	{
		auto bytes_readed = co_await beast::http::async_read_some(sock_archive, buf, parser_archive_response, asio::use_awaitable);
		auto & data = parser_archive_response.get().body();
		fmt::print("[reader] - read {} bytes.\n", bytes_readed);
		response_user.body().data = data.data();
		response_user.body().size = data.size();
		auto [err, bytes_writed] = co_await beast::http::async_write(stream_user, serializer_user, asio::as_tuple(asio::use_awaitable));
		if(err.failed() && err != beast::http::error::need_buffer) throw err;
		fmt::print("[writer] - Write {} bytes.\n", bytes_writed);
		data.clear();
	}

	// Close user connection.
    stream_user.socket().shutdown(asio::ip::tcp::socket::shutdown_send);
}

auto listener() -> asio::awaitable<void>
{
	auto ctx = co_await this_coro::executor;
	asio::ip::tcp::acceptor acceptor {ctx, {asio::ip::tcp::v4(), 80}};
    for(;;)
	{
		beast::tcp_stream tcp_stream {co_await acceptor.async_accept(asio::use_awaitable)};
		asio::co_spawn(ctx, session(std::move(tcp_stream)), hf::rethrowed);
	}
}

int main() try
{
	SetThreadUILanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
    asio::io_context ctx;
    asio::co_spawn(ctx, listener(), hf::rethrowed);
    return ctx.run();
} catch(std::exception e) { fmt::print("Except: '{}'.\n", e.what()); }
