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

	std::string string;
	auto dynamic_buffer = asio::dynamic_buffer(string);

	for(std::string cmd; std::getline(std::cin, cmd);)
	{
		if(cmd == "read")
		{
			std::size_t readed = co_await asio::async_read(socket, dynamic_buffer, asio::transfer_at_least(1), io::use_coro);
			asio::const_buffer buffer = dynamic_buffer.data(0, readed);
			fmt::print("readed {} octets, message: '{}'.\n", readed, std::string(static_cast<const char *>(buffer.data()), buffer.size()));
			dynamic_buffer.consume(readed);
		}
		if(cmd == "read_until")
		{
			std::size_t readed = co_await asio::async_read_until(socket, dynamic_buffer, '0', io::use_coro);
			asio::const_buffer buffer = dynamic_buffer.data(0, readed);
			fmt::print("readed {} octets, message: '{}'.\n", readed, std::string(static_cast<const char *>(buffer.data()), buffer.size()));
			dynamic_buffer.consume(readed);
		}
		if(cmd == "show_buffer")
		{
			asio::const_buffer buffer = dynamic_buffer.data(0, dynamic_buffer.size());
			fmt::print("buffer: '{}'.\n", std::string(static_cast<const char *>(buffer.data()), buffer.size()));
		}
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
