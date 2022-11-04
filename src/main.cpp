#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>
#include <fmt/core.h>
#include <httpfox.hpp>
#include <fastdevhax.hpp>
#include <unordered_map>
#include <vector>
#include <map>
#include <jaja_notation.hpp>
#include <set>
#include <unordered_set>

// template <typename T> class context_local
// {
// 	prv T var;

// 	pbl auto value()
// 	{

// 	}
// };

std::unordered_map<asio::any_io_executor, int, decltype([](auto...){ return 1; })> m;

auto coro_a() -> asio::awaitable<void>
{
	m[co_await this_coro::executor]++;
	fmt::print("[coro 'a'] - Value: '{}'.\n", m[co_await this_coro::executor]);
}

auto coro_b() -> asio::awaitable<void>
{
	m[co_await this_coro::executor]++;
	fmt::print("[coro 'b'] - Value: '{}'.\n", m[co_await this_coro::executor]);
}

int main()
{
	asio::io_context ctx_a;
	asio::io_context ctx_b;

	for(int i = 0; i < 10; i++) asio::co_spawn(ctx_a, coro_a(), hf::rethrowed);
	for(int i = 0; i < 10; i++) asio::co_spawn(ctx_b, coro_b(), hf::rethrowed);

	ctx_a.run();
	ctx_b.run();

	return 0;
}
