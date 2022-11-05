#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/experimental/use_coro.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/this_coro.hpp>
#include <chrono>
#include <fmt/core.h>
#include <functional>
#include <httpfox.hpp>
#include <fastdevhax.hpp>
#include <optional>
#include <unordered_map>
#include <vector>
#include <map>
#include <jaja_notation.hpp>
#include <set>
#include <unordered_set>
#include <boost/asio/experimental/coro.hpp>
#include <boost/asio/experimental/co_spawn.hpp>

auto sub_coro(auto & executor) -> asio::experimental::coro<void, int>
{
	co_await asio::steady_timer(executor, std::chrono::seconds(5)).async_wait(asio::experimental::use_coro);
	co_return 10;
}

auto incremental_coro(auto & executor) -> asio::experimental::coro<int(int)>
{
	for(int i = 5;; i++)
	{
		co_await asio::steady_timer(executor, std::chrono::seconds(1)).async_wait(asio::experimental::use_coro);
		co_yield i;
	}
}

asio::experimental::coro<int(int)> my_sum(asio::io_context & x)
{
	for(int i = 5;; i++)
	{
		co_await asio::steady_timer(x, std::chrono::seconds(1)).async_wait(asio::experimental::use_coro);
		co_yield i;
	}
}

auto coro(auto & executor) -> asio::experimental::coro<void()>
{
	boost::asio::execution_context context;
	boost::asio::execution_context::id x;

	// fmt::print("[coro] - co_await test start.\n");
	// co_await asio::steady_timer(executor, std::chrono::seconds(5)).async_wait(asio::experimental::use_coro);
	// fmt::print("[coro] - co_await test done.\n");

	// auto x = co_await sub_coro(executor);
	// fmt::print("[coro] - co_return test: '{}'\n", x);

	// std::optional<int> x = co_await incremental_coro(executor);
	// for(;;)
	// {
	// 	std::optional<int> x = co_await x.value();
	// 	fmt::print("[coro] - co_yield test: '{}'\n", x.value());
	// }

	asio::io_context c;
	auto sum = my_sum(c);
	for(;;)
	{
		std::optional<int> x = co_await sum(5);
		fmt::print("[coro] - co_yield test: '{}'\n", x.value());
	}

	co_return;
}

int main()
{
	asio::io_context ctx;
	asio::experimental::co_spawn(coro(ctx), asio::detached);
	ctx.run();
	return 0;
}
