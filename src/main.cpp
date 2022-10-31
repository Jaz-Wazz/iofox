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
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/write.hpp>
#include <fmt/core.h>
#include <iostream>
#include <httpfox.hpp>

namespace asio = boost::asio;					// NOLINT
namespace beast = boost::beast;					// NOLINT
namespace this_coro = boost::asio::this_coro;	// NOLINT

asio::awaitable<void> session(beast::tcp_stream stream)
{
	// Get request.
    beast::flat_buffer buf;
	beast::http::request<beast::http::string_body> req;
	co_await beast::http::async_read(stream, buf, req, asio::use_awaitable);
	std::cout << req;

	// Send response.
	beast::http::response<beast::http::string_body> res;
	res.body() = "hello, a ne idi naxui.";
	co_await beast::http::async_write(stream, res, asio::use_awaitable);

	// Disconnect.
    stream.socket().shutdown(asio::ip::tcp::socket::shutdown_send);
}

asio::awaitable<void> listener()
{
	auto ctx = co_await this_coro::executor;
	asio::ip::tcp::acceptor acceptor {ctx, {asio::ip::tcp::v4(), 80}};
    for(;;)
	{
		beast::tcp_stream tcp_stream {co_await acceptor.async_accept(asio::use_awaitable)};
		asio::co_spawn(ctx, session(std::move(tcp_stream)), hf::rethrowed);
	}
}

asio::awaitable<void> test()
{
	auto ctx = co_await this_coro::executor;

	boost::asio::ip::tcp::resolver resolver {ctx};
	auto resolver_results = co_await resolver.async_resolve("archive.org", "http", asio::use_awaitable);

	asio::ip::tcp::socket sock {ctx};
	co_await asio::async_connect(sock, resolver_results, asio::use_awaitable);

	beast::http::request<beast::http::empty_body> request {beast::http::verb::get, "/download/stream_archive/main.html", 11};
	request.set("Host", "archive.org");
	co_await beast::http::async_write(sock, request, asio::use_awaitable);
	std::cout << request << '\n';

	beast::flat_buffer buffer;
	beast::http::response<beast::http::string_body> response;
	co_await beast::http::async_read(sock, buffer, response, asio::use_awaitable);
	std::cout << response << '\n';
}

int main()
{
    asio::io_context ctx;
    asio::co_spawn(ctx, test(), hf::rethrowed);
    return ctx.run();
}
