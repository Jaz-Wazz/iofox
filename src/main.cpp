#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/registered_buffer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <fmt/core.h>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <iofox/iofox.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto session(asio::ip::tcp::socket socket) -> io::coro<void>
{
	fmt::print("connected.\n");
	char buffer[8192] {};
	std::size_t buffer_size = 0;
	std::size_t prev_buffer_size = 0;

	for(std::string cmd; std::getline(std::cin, cmd);)
	{
		if(cmd == "read")
		{
			std::size_t readed = co_await socket.async_read_some(asio::buffer(buffer + buffer_size, sizeof(buffer) - buffer_size), io::use_coro);

			auto buffer_0 = asio::buffer(buffer, buffer_size);
			auto buffer_1 = asio::buffer(buffer + buffer_size, readed);
			auto buffer_2 = asio::buffer(buffer + buffer_size + readed, sizeof(buffer) - buffer_size - readed);
			buffer_size += readed;

			fmt::print("┌─────────────────────────────────────────────────────────────────────────────────────────┐\n");
			fmt::print("│ Buffer dump                                                                             │\n");
			fmt::print("├───────────┬────────┬─────────────────────────────────────────────────┬──────────────────┤\n");

			for(auto chunk : io::log::hex_dump(buffer_0.data(), buffer_0.size()) | std::views::take(1))
			{
				fmt::print("│ Buffer 0: │ {} │ {} │ {} │\n", chunk.offset(), chunk.bytes(), chunk.chars());
			}

			for(auto chunk : io::log::hex_dump(buffer_0.data(), buffer_0.size()) | std::views::drop(1))
			{
				fmt::print("│           │ {} │ {} │ {} │\n", chunk.offset(), chunk.bytes(), chunk.chars());
			}

			fmt::print("├───────────┼────────┼─────────────────────────────────────────────────┼──────────────────┤\n");

			for(auto chunk : io::log::hex_dump(buffer_1.data(), buffer_1.size()) | std::views::take(1))
			{
				fmt::print("│ Buffer 1: │ {} │ {} │ {} │\n", chunk.offset(), chunk.bytes(), chunk.chars());
			}

			for(auto chunk : io::log::hex_dump(buffer_1.data(), buffer_1.size()) | std::views::drop(1))
			{
				fmt::print("│           │ {} │ {} │ {} │\n", chunk.offset(), chunk.bytes(), chunk.chars());
			}

			fmt::print("├───────────┼────────┼─────────────────────────────────────────────────┼──────────────────┤\n");

			auto dump_2 = io::log::hex_dump(buffer_2.data(), buffer_2.size());

			for(auto chunk : dump_2 | std::views::take(1))
			{
				fmt::print("│ Buffer 2: │ {} │ {} │ {} │\n", chunk.offset(), chunk.bytes(), chunk.chars());
			}

			for(auto chunk : dump_2 | std::views::drop(1) | std::views::take(4))
			{
				fmt::print("│           │ {} │ {} │ {} │\n", chunk.offset(), chunk.bytes(), chunk.chars());
			}
			fmt::print("│           │        │ {:47} │                  │\n", fmt::format("And {} same lines...", dump_2.size() - 5));
			fmt::print("└───────────┴────────┴─────────────────────────────────────────────────┴──────────────────┘\n");
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
