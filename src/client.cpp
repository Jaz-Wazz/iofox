#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/read.hpp>
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
		// fmt::print("[reader 'a'] - readed: {:16} bytes, size: {:16} bytes.\n", readed, size);
	}

	auto finish = std::chrono::steady_clock::now();
	fmt::print("Time 'reader a': {}.\n", std::chrono::duration_cast<std::chrono::milliseconds>(finish - start));
	// std::ofstream("reader_a.txt").write(data, size);
};

auto reader_b(asio::ip::tcp::socket socket) -> io::coro<void>
{
	char * data = new char[528888890];

	auto start = std::chrono::steady_clock::now();
	std::size_t readed = co_await asio::async_read(socket, asio::buffer(data, 528888890), io::use_coro);
	// fmt::print("[reader 'b'] - readed: {} bytes.\n", readed);
	auto finish = std::chrono::steady_clock::now();

	fmt::print("Time 'reader b': {}.\n", std::chrono::duration_cast<std::chrono::milliseconds>(finish - start));
	// std::ofstream("reader_b.txt").write(data, 528888890);
};

auto reader_c(asio::ip::tcp::socket socket) -> io::coro<void>
{
	char * data = new char[528888890];
	std::size_t size = 0;

	auto start = std::chrono::steady_clock::now();
	for(;size != 528888890;)
	{
		auto buffer = asio::buffer(data + size, (528888890 - size > 8192) ? 8192 : 528888890 - size);
		std::size_t readed = co_await socket.async_read_some(buffer, io::use_coro);
		size += readed;
		// fmt::print("[reader 'c'] - readed: {:16} bytes, size: {:16} bytes.\n", readed, size);
	}
	auto finish = std::chrono::steady_clock::now();

	fmt::print("Time 'reader c': {}.\n", std::chrono::duration_cast<std::chrono::milliseconds>(finish - start));
	// std::ofstream("reader_c.txt").write(data, 528888890);
};

auto open_session() -> io::coro<asio::ip::tcp::socket>
{
	asio::ip::tcp::socket socket {co_await this_coro::executor};
	co_await socket.async_connect({asio::ip::make_address("127.0.0.1"), 555}, io::use_coro);
	co_return socket;
}

auto coro() -> io::coro<void>
{
	co_await reader_a(co_await open_session());
	co_await reader_b(co_await open_session());
	co_await reader_c(co_await open_session());
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
