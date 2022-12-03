#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <chrono>
#include <fmt/core.h>
#include <net_tails.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto foo() -> nt::result<void>
{
	// nt::service<int> service;
	// auto x = co_await service.get_or_make(10);
	// nt_check(x);

	// auto && executor = co_await this_coro::executor;
	// auto y = co_await asio::steady_timer(executor, 5s).async_wait(nt::use_result);
	// nt_check(y);

	// if(auto ret = co_await nt::async_try(foo))
	// {

	// }
	// {

	// }

	// nt::async_catch(send_page, [](std::exception & e)
	// {
	// });

	//

	co_return nt::success();
}

auto coro() -> nt::result<void>
{
	auto hosts = nt_async(nt::dns::resolve("http", "exmaple.com"));
	for(auto host : hosts) fmt::print("{}.\n", host.host_name());
	co_return nt::success();
}

int main()
{
	asio::io_context ctx;
	asio::co_spawn(ctx, coro(), [](std::exception_ptr ptr)
	{
		if(ptr)
		{
			try { std::rethrow_exception(ptr); }
			catch(std::exception & e) { fmt::print("Exception: '{}'.\n", e.what()); }
			catch(...) { fmt::print("Exception: 'unknown'.\n"); }
		}
	});
	return ctx.run();
}
