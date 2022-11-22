#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/leaf/error.hpp>
#include <boost/leaf/handle_errors.hpp>
#include <boost/leaf/result.hpp>
#include <chrono>
#include <exception>
#include <fmt/core.h>
#include <stdexcept>

namespace leaf = boost::leaf;			// NOLINT
namespace asio = boost::asio;			// NOLINT
namespace this_coro = asio::this_coro;	// NOLINT

namespace netfox
{
	auto try_catch(auto block_try, auto block_catch) -> asio::awaitable<void>
	{
		std::exception_ptr hax = nullptr;
		std::exception * x = nullptr;
		try
		{
			co_await block_try();
		}
		catch(std::exception & e)
		{
			hax = std::current_exception();
			x = &e;
		}
		if(x != nullptr) co_await block_catch(*x);
	}
}

auto coro() -> asio::awaitable<void>
{
	co_await netfox::try_catch([] -> asio::awaitable<void>
	{
		throw std::runtime_error("sas");
	}, [](std::exception & e) -> asio::awaitable<void>
	{
		co_await asio::steady_timer(co_await this_coro::executor, std::chrono::seconds(5)).async_wait(asio::use_awaitable);
		fmt::print("err: '{}'.\n", e.what());
	});

	co_return;
}

int main()
{
	asio::io_context ctx;
	asio::co_spawn(ctx, coro(), asio::detached);
	return ctx.run();
}
