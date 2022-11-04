#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/this_coro.hpp>
#include <fmt/core.h>
#include <functional>
#include <httpfox.hpp>
#include <fastdevhax.hpp>
#include <unordered_map>
#include <vector>
#include <map>
#include <jaja_notation.hpp>
#include <set>
#include <unordered_set>
#include <boost/asio/experimental/coro.hpp>

// [Это расчитано на небольшое количество контекстов]
template <typename T> class context_local
{
	prv std::unordered_map<asio::any_io_executor, T, decltype([](auto...){ return 1; })> m;

	pbl auto value() -> asio::awaitable<std::reference_wrapper<T>>
	{
		for(auto & el : m)
		{
			bool x = el.first;
			fmt::print("> [{}]\n", x);
		}
		// fmt::print("{}.\n", !!m[co_await this_coro::executor]);
		co_return m[co_await this_coro::executor];
	}

	pbl void clear() {}
};

context_local<int> var;

auto coro_a() -> asio::awaitable<void>
{
	int & x = co_await var.value();
	// bool test = co_await this_coro::executor;
	fmt::print("[coro 'a'] - Value: '{}'.\n", x++);
}

// auto coro_b() -> asio::awaitable<int>
// {
// 	int & x = co_await var.value();
// 	// bool test = co_await this_coro::executor;
// 	fmt::print("[coro 'b'] - Value: '{}'.\n", x++);
// }

auto coro(auto executor) -> asio::experimental::coro<void(int)>
{
	co_yield 3;
	co_return 42;
}

int main()
{
	{
		asio::io_context ctx;
		for(int i = 0; i < 10; i++) asio::co_spawn(ctx, coro_a(), hf::rethrowed);
		ctx.run();
	}
	{
		asio::io_context ctx;
		for(int i = 0; i < 10; i++) asio::co_spawn(ctx, coro_b(), hf::rethrowed);
		ctx.run();
	}

	return 0;
}
