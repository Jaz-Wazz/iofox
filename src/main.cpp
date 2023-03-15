#include <boost/asio/buffer.hpp>
#include <boost/asio/buffered_read_stream.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/this_coro.hpp>
#include <fmt/core.h>
#include <iostream>
#include <string>
#include <vector>
#include <iofox/iofox.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffered_stream.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto session(asio::ip::tcp::socket socket) -> io::coro<void>
{
	fmt::print("Connected.\n");
	asio::buffered_read_stream<asio::ip::tcp::socket> buffered_socket {socket.get_executor()};
	buffered_socket.lowest_layer() = std::move(socket);

	for(std::string cmd; std::getline(std::cin, cmd);)
	{
		if(cmd == "peak")
		{
			std::string peaked_buffer;
			peaked_buffer.resize(100);
			std::size_t peaked = buffered_socket.peek(asio::buffer(peaked_buffer));
			fmt::print("Peak operation: '{}' octets, '{}' data.\n", peaked, std::string(peaked_buffer.c_str(), peaked));
		}
		if(cmd == "fill")
		{
			std::size_t filled = co_await buffered_socket.async_fill(io::use_coro);
			fmt::print("Filled operation: '{}' octets.\n", filled);
		}
		if(cmd == "read")
		{
			std::string buffer;
			std::size_t readed = co_await asio::async_read(buffered_socket, asio::dynamic_buffer(buffer), asio::transfer_at_least(1), io::use_coro);
			fmt::print("Readed: {} octets: '{}'.\n", readed, buffer);
		}
	}

	fmt::print("Disconnected.\n");
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
