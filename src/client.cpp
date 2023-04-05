#include <boost/asio/async_result.hpp>
#include <boost/asio/bind_allocator.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/experimental/co_composed.hpp>
#include <boost/system/detail/error_code.hpp>
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

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

template <asio::completion_token_for<void(char)> CompletionToken>
auto async_foo_a(CompletionToken && token)
{
	return asio::async_initiate<CompletionToken, void(char)>([=](auto completion_handler)
	{
		completion_handler('a');
	}, token);
}

template <asio::completion_token_for<void(char)> CompletionToken>
auto async_foo_b(CompletionToken && token)
{
	return asio::async_initiate<CompletionToken, void(char)>([=](auto completion_handler)
	{
		boost::asio::post([completion_handler = std::move(completion_handler)] mutable
		{
			completion_handler('b');
		});
	}, token);
}

template <asio::completion_token_for<void(char)> CompletionToken>
auto async_foo_c(auto & ctx, CompletionToken && token)
{
	return asio::async_initiate<CompletionToken, void(char)>([=](auto completion_handler)
	{
		boost::asio::post(ctx, [completion_handler = std::move(completion_handler)] mutable
		{
			completion_handler('c');
		});
	}, token);
}

template <typename CompletionToken>
auto async_foo_d(CompletionToken && token)
{
	return asio::async_compose<CompletionToken, void(char)>([](auto & self)
	{
		self.complete('d');
	}, token);
}

struct async_f_implementation
{
	template <typename Self>
	void operator()(Self & self, boost::system::error_code error = {}, std::size_t n = 0)
	{
		self.complete(error, 15);
	}
};

template <typename CompletionToken>
auto async_foo_f(CompletionToken&& token)
-> decltype(asio::async_compose<CompletionToken, void(boost::system::error_code, std::size_t)>(std::declval<async_f_implementation>(), token))
{
	return boost::asio::async_compose<CompletionToken, void(boost::system::error_code, std::size_t)>(async_f_implementation{}, token);
}

template <typename CompletionToken>
auto async_foo_g(CompletionToken&& token)
{
	return asio::async_initiate<CompletionToken, void(boost::system::error_code)>(
		asio::experimental::co_composed([](auto state) -> void
        {
			state.reset_cancellation_state(boost::asio::enable_terminal_cancellation());
			boost::system::error_code error = {};
            co_yield state.complete(error);
        }), token);
}

auto coro() -> io::coro<void>
{
	// for(;;) async_foo_a([](char c){ fmt::print("{}", c); });
	// for(;;) fmt::print("{}", co_await async_foo_a(io::use_coro));

	// for(;;) async_foo_b([](char c){ fmt::print("{}", c); });
	// for(;;) fmt::print("{}", co_await async_foo_b(io::use_coro));

	// auto ctx = co_await this_coro::executor;
	// for(;;) async_foo_c(ctx, [](char c){ fmt::print("{}", c); });
	// for(;;) fmt::print("{}", co_await async_foo_c(ctx, io::use_coro));

	// for(;;) async_foo_d([](char c){ fmt::print("{}", c); });
	// for(;;) fmt::print("{}", co_await async_foo_d(io::use_coro));

	// for(;;) async_foo_f([](boost::system::error_code ec, std::size_t c){ fmt::print("{}", c); });
	// for(;;) fmt::print("{}", co_await async_foo_f(io::use_coro));
	// for(;;) std::size_t i = co_await async_foo_f(io::use_coro);

	// for(;;) async_foo_g([](boost::system::error_code ec){ fmt::print("g"); });
	// for(;;) co_await async_foo_g(io::use_coro);

	co_return;
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
