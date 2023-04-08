#include <boost/asio/async_result.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/defer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/experimental/deferred.hpp>
#include <boost/asio/experimental/coro_traits.hpp>
#include <boost/asio/experimental/coro.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/experimental/co_composed.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/use_future.hpp>
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
#include <utility>
#include <boost/asio.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto test_a() -> asio::awaitable<void>
{
	asio::steady_timer timer {co_await asio::this_coro::executor, std::chrono::seconds(5)};
	auto deferred_operation = timer.async_wait(asio::deferred);
	co_await std::move(deferred_operation);
}

auto test_b() -> asio::awaitable<void>
{
	asio::steady_timer timer {co_await asio::this_coro::executor, std::chrono::seconds(5)};
	co_await [&](auto && token)
	{
		return timer.async_wait(io::use_coro);
	};
}

// auto test_c() -> asio::awaitable<void>
// {
// 	asio::steady_timer timer {co_await asio::this_coro::executor, std::chrono::seconds(5)};
// 	auto future = timer.async_wait(asio::use_future);
// 	co_await future;
// }

// auto test_d() -> asio::awaitable<void>
// {
// 	asio::steady_timer timer {co_await asio::this_coro::executor, std::chrono::seconds(5)};
// 	asio::async_operation<void(boost::system::error_code)> auto my_op = [&](auto && token) {return timer.async_wait(std::move(token));};
// }

// auto test_x() -> asio::awaitable<void>
// {
// 	auto x = asio::deferred.when(true).then(asio::deferred.values('x')).otherwise(asio::deferred.values('y'));

// 	std::move(x)([](char c){ /* char */ });
// 	auto c = co_await std::move(x)(io::use_coro);

// 	// co_await x;

// 	co_return;
// }

int main() try
{
	io::windows::set_asio_locale(io::windows::lang::english);
	asio::io_context ctx;
	// asio::co_spawn(ctx, coro_x(), io::rethrowed);
	return ctx.run();
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
