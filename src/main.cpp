#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/write.hpp>
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

auto session(beast::tcp_stream stream) -> asio::awaitable<void>
{
    beast::flat_buffer buf;
	beast::http::request<beast::http::string_body> req;
	co_await beast::http::async_read(stream, buf, req, asio::use_awaitable);
	auto page = req.target().to_string();

	if(auto result = co_await take_archive_location(page))
	{
		auto [host, path] = result.value();

		if(auto result = co_await http_get_contents(host, path))
		{
			beast::http::response<beast::http::string_body> res;
			res.body() = result.value();
			co_await beast::http::async_write(stream, res, asio::use_awaitable);
			fmt::print("[haxserv] - [{}] -> [{}{}] ({}).\n", page, host, path, result.value().size());
		}
		else
		{
			beast::http::response<beast::http::string_body> res {beast::http::status::not_found, 11};
			co_await beast::http::async_write(stream, res, asio::use_awaitable);
			fmt::print("[haxserv] - [{}] -> [404] (Get data error, code: '{}').\n", page, result.error());
		}
	}
	else
	{
		beast::http::response<beast::http::string_body> res {beast::http::status::not_found, 11};
		co_await beast::http::async_write(stream, res, asio::use_awaitable);
		fmt::print("[haxserv] - [{}] -> [404] (Taking location error, code: '{}').\n", page, result.error());
	}

    stream.socket().shutdown(asio::ip::tcp::socket::shutdown_send);
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
