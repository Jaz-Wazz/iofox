#include <boost/asio/co_spawn.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/experimental/coro.hpp>
#include <boost/asio/experimental/use_coro.hpp>
#include <chrono>
#include <cstring>
#include <fmt/core.h>
#include <fmt/chrono.h>
#include <exception>
#include <fstream>
#include <iofox/iofox.hpp>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <variant>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto reader_e(io::isstream stream) -> io::coro<void>
{
	char * data = new char[528888890];
	std::size_t size = 0;

	for(;size != 528888890; size++)
	{
		if(auto chr = stream.get())
		{
			data[size] = chr.value();
		}
		else
		{
			co_await stream.async_fill();
			data[size] = stream.get().value();
		}
	}
}

auto open_session() -> io::coro<asio::ip::tcp::socket>
{
	asio::ip::tcp::socket socket {co_await this_coro::executor};
	co_await socket.async_connect({asio::ip::make_address("127.0.0.1"), 555}, io::use_coro);
	co_return socket;
}

auto benchmark(auto title, auto callable) -> io::coro<void>
{
	auto start = std::chrono::steady_clock::now();
	co_await callable(co_await open_session());
	auto finish = std::chrono::steady_clock::now();
	fmt::print("Benchmark '{}': {}.\n", title, std::chrono::duration_cast<std::chrono::milliseconds>(finish - start));
}

auto coro() -> io::coro<void>
{
	co_await benchmark("reader_e", reader_e);
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
