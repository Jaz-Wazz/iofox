#include <array>
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
#include <string_view>
#include <utility>
#include <vector>
#include <iofox/iofox.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffered_stream.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto session(asio::ip::tcp::socket socket) -> io::coro<void>
{
	fmt::print("connected.\n");
	char buffer[32] {};
	std::size_t buffer_size = 0;

	for(std::string cmd; std::getline(std::cin, cmd);)
	{
		if(cmd == "read")
		{
			std::size_t readed = co_await socket.async_read_some(asio::buffer(buffer + buffer_size, sizeof(buffer) - buffer_size), io::use_coro);
			fmt::print("buffer: '{}'.\n", std::string(buffer, buffer_size + readed));
			fmt::print("readed: '{}'.\n", std::string(buffer + buffer_size, readed));
			buffer_size += readed;
		}
	}
}

auto coro() -> io::coro<void>
{
	asio::ip::tcp::acceptor acceptor {co_await this_coro::executor, {asio::ip::tcp::v4(), 555}};
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
