#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <chrono>
#include <fmt/core.h>
#include <fmt/chrono.h>
#include <exception>
#include <fstream>
#include <iofox/iofox.hpp>
#include <ranges>
#include <string_view>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto reader_a(asio::ip::tcp::socket socket) -> io::coro<void>
{
	char * data = new char[528888890];
	std::size_t size = 0;

	auto start = std::chrono::steady_clock::now();

	for(;size != 528888890;)
	{
		std::size_t readed = co_await socket.async_read_some(asio::buffer(data + size, 528888890 - size), io::use_coro);
		size += readed;
		fmt::print("[reader 'a'] - readed: {:16} bytes, size: {:16} bytes.\n", readed, size);
	}

	auto finish = std::chrono::steady_clock::now();
	fmt::print("Time: {}.\n", std::chrono::duration_cast<std::chrono::milliseconds>(finish - start));

	// std::ofstream("test_1.txt").write(data, size);
};

auto coro() -> io::coro<void>
{
	asio::ip::tcp::socket socket {co_await this_coro::executor};
	co_await socket.async_connect({asio::ip::make_address("127.0.0.1"), 555}, io::use_coro);
	fmt::print("[client] - connected.\n");

	co_await reader_a(std::move(socket));
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
