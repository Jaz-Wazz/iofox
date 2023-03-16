#include <boost/asio/buffer.hpp>
#include <boost/asio/buffered_read_stream.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/this_coro.hpp>
#include <cstring>
#include <fmt/core.h>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <iofox/iofox.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffered_stream.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

class my_stream : private asio::ip::tcp::socket
{
	public: my_stream(asio::ip::tcp::socket && socket): asio::ip::tcp::socket(std::move(socket)) {}
	public: auto get_executor() { return asio::ip::tcp::socket::get_executor(); }

	public: auto async_read_some(const auto & buffers, auto && token)
	{
		return asio::ip::tcp::socket::async_read_some(buffers, std::move(token));
	}
};

auto session(asio::ip::tcp::socket socket) -> io::coro<void>
{
	my_stream my_stream {std::move(socket)};
	fmt::print("Connected.\n");
	for(;;)
	{
		std::string string;
		string.resize(4);
		std::size_t readed = co_await my_stream.async_read_some(asio::buffer(string), io::use_coro);
		fmt::print("Readed: {} octets, message: '{}'.\n", readed, string);
	}
}

auto coro() -> io::coro<void>
{
	asio::ip::tcp::acceptor acceptor {co_await this_coro::executor, {asio::ip::tcp::v4(), 80}};
    for(;;) asio::co_spawn(co_await this_coro::executor, session(co_await acceptor.async_accept(asio::use_awaitable)), io::rethrowed);
}

int main() try
{
	io::windows::set_asio_locale(io::windows::lang::english);
	asio::io_context ctx;
	asio::co_spawn(ctx, coro(), io::rethrowed);
	return ctx.run();
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
