#include <boost/asio/bind_allocator.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/recycling_allocator.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/read.hpp>
#include <chrono>
#include <cstring>
#include <fmt/core.h>
#include <fmt/chrono.h>
#include <exception>
#include <fstream>
#include <iofox/iofox.hpp>
#include <iostream>
#include <locale>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto sync_get(char c)
{
	return c;
}

auto async_get_a(char c) -> io::coro<char>
{
	co_return c;
}

auto async_get_b(char c, asio::completion_token_for<void(char)> auto && token)
{
	return asio::async_initiate<decltype(token), void(char)>([=](auto completion_handler)
	{
		boost::asio::post([c, completion_handler = std::move(completion_handler)] mutable
		{
			completion_handler(c);
		});
	}, token);
}

auto async_get_c(char c, auto & ctx, asio::completion_token_for<void(char)> auto && token)
{
	return asio::async_initiate<decltype(token), void(char)>([=](auto completion_handler)
	{
		boost::asio::post(ctx, [c, completion_handler = std::move(completion_handler)] mutable
		{
			completion_handler(c);
		});
	}, token);
}

auto async_get_d(char c, auto & ctx, auto && token)
{
	return asio::co_spawn(ctx, [&] -> io::coro<char>
	{
		co_return c;
	}, std::forward<decltype(token)>(token));
}

auto async_get_e(char c, auto & ctx, auto && token)
{
	return asio::async_compose<decltype(token), void(char)>([&](auto&& self)
	{
		boost::asio::post(ctx, [c, self = std::move(self)] mutable
		{
			self.complete(c);
		});
	}, token);
}

auto reader_g(asio::ip::tcp::socket socket) -> io::coro<void>
{
	auto ctx = co_await this_coro::executor;
	asio::recycling_allocator<char> alloc;

	// Object buffer.
	volatile char * data = new char[528888890];
	std::size_t size = 0;

	// Internal buffer.
	char internal_buffer[4096] {};
	std::size_t internal_buffer_size = 0;

	for(;size != 528888890;)
	{
		internal_buffer_size = co_await socket.async_read_some(asio::buffer(internal_buffer), io::use_coro);

		for(int i = 0; char c : std::span(internal_buffer, internal_buffer_size))
		{
			// stream.buffer.getc();
			// stream.getc();

			// stream.getc();
			// stream.async_getc([](char c){});

			// data[size + i] = sync_get(c);
			// data[size + i] = co_await async_get_a(c);
			// data[size + i] = co_await async_get_b(c, io::use_coro);
			// data[size + i] = co_await async_get_c(c, ctx, io::use_coro);
			// data[size + i] = co_await async_get_d(c, ctx, io::use_coro);
			// data[size + i] = co_await async_get_e(c, ctx, io::use_coro);
			async_get_e(c, ctx, [=](char c){ data[size + i] = c;});

			// auto x = asio::bind_allocator(alloc, asio::use_awaitable);
			// auto x = asio::bind_allocator(alloc, async_get_a('x'));

			// data[size + i] = co_await async_get_d(c, ctx, asio::bind_allocator(alloc, asio::use_awaitable));
		}

		size += internal_buffer_size;
		fmt::print("[reader_f] - read: {:16}, size: {:16L}.\n", internal_buffer_size, size);
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
	co_await benchmark("reader_g", reader_g);
}

int main() try
{
	std::locale::global(std::locale("en_US.UTF-8"));
	io::windows::set_asio_locale(io::windows::lang::english);
	asio::io_context ctx;
	asio::co_spawn(ctx, coro(), io::rethrowed);
	return ctx.run();
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
