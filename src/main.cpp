#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/experimental/coro.hpp>
#include <boost/asio/experimental/co_spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/outcome.hpp>
#include <coroutine>
#include <fmt/core.h>
#include <string>

namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;	// NOLINT.
namespace asio = boost::asio;					// NOLINT.

auto some_foo() -> asio::awaitable<int>
{
	co_return 5;
}

auto foo() -> asio::awaitable<asio::awaitable<int>>
{
	asio::awaitable<int> aw = some_foo();
	co_return aw;
}

auto coro() -> asio::awaitable<void>
{
	auto x = co_await foo();
	auto y = co_await std::move(x);
	fmt::print("Value: '{}'.\n", y);

	auto k = co_await co_await foo();
	fmt::print("Value: '{}'.\n", y);
}

int main()
{
	asio::io_context ctx;
	asio::co_spawn(ctx, coro(), asio::detached);
	return ctx.run();
}
