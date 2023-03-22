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
	std::array<char, 10> buffer {};
	std::string out;

	for(;;)
	{
		std::size_t readed = co_await socket.async_read_some(asio::buffer(buffer), io::use_coro);
		fmt::print("readed: '{}'.\n", std::string(buffer.data(), readed));
		for(int i = 0; i < readed; i++)
		{
			if(buffer[i] != 'x') out.push_back(buffer[i]); else goto exit;
		}
	} exit:

	fmt::print("out string: '{}'.\n", out);
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
