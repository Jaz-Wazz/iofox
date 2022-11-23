#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/detail/handler_work.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/system_executor.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/outcome.hpp>
#include <boost/outcome/outcome.hpp>
#include <boost/outcome/result.hpp>
#include <boost/outcome/success_failure.hpp>
#include <boost/system/detail/errc.hpp>
#include <exception>
#include <fmt/core.h>
#include <stdexcept>
#include <system_error>
#include <as_result.hpp>

namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;	// NOLINT
namespace asio = boost::asio;					// NOLINT
namespace this_coro = asio::this_coro;			// NOLINT
using namespace std::chrono_literals;


// auto foo() -> asio::awaitable<void>
// {
// 	fmt::print("[foo] - wait 5 sec.\n");
// 	co_await asio::steady_timer(co_await this_coro::executor, 5s).async_wait(asio::use_awaitable);
// 	fmt::print("[foo] - wait ended.\n");
// }

#define ef else if

auto foo_a() -> asio::awaitable<outcome::outcome<std::string, std::error_code>>
{
	co_return "string";
	// co_return std::errc::address_family_not_supported;
	// try { throw std::runtime_error("sas"); } catch(...) { co_return std::current_exception(); }
}

auto coro() -> asio::awaitable<void>
{
	// if(auto x = co_await foo_a())
	// {
	// 	fmt::print("Succes: '{}'.\n", x.value());
	// }
	// ef(x.has_exception())
	// {
	// 	fmt::print("Except.\n");
	// }
	// else
	// {
	// 	fmt::print("Fuck: '{}'.\n", x.error().message());
	// }

	// // auto x = co_await asio::steady_timer(co_await this_coro::executor, 5s).async_wait(as_result(asio::use_awaitable));
	// // fmt::print("[foo] - wait ended.\n");

	// try
	// {
	// 	// async_connect();
	// 	// async_write();
	// 	// async_read();
	// 	// async_disconnect();
	// }
	// catch(...)
	// {
	// 	// print_error();
	// 	// async_disconnect();
	// }

	// auto ret = [] -> outcome::outcome<void, std::error_code, std::exception_ptr>
	// {
	// 	// TRY(async_connect());
	// 	// TRY(async_write());
	// 	// TRY(async_read());
	// 	// TRY(async_disconnect());
	// 	return outcome::success();
	// }();

	// if(ret.has_failure())
	// {
	// 	// async_disconnect();
	// }
	// if(ret.has_exception())
	// {
	// 	// async_disconnect();

	// 	try { std::rethrow_exception(ret.exception()); } catch(std::exception & e)
	// 	{
	// 		// Handle exception.
	// 	}
	// }

	// co_return;

	// auto r = []
	// {
	// 	// async_connect();
	// 	// async_write();
	// 	// async_read();
	// 	// async_disconnect();
	// };

	// std::exception_ptr ptr;
	// try { r(); } catch(...) { ptr = std::current_exception(); }

	// if(ptr)
	// {
	// 	// async_disconnect();
	// }

	// try { std::rethrow_exception(ptr); } catch(std::exception & e)
	// {
	// 	// Handle exception.
	// }

	//  netfox_try(client.connect("http", "google.com"));

	auto executor = co_await this_coro::executor;

	try
	{
		int i = 10;
		throw std::runtime_error("sas");
	}
	catch(std::exception & e)
	{
		fmt::print("except.\n");
		asio::co_spawn(executor, [] -> asio::awaitable<void>
		{
			fmt::print("[error handler] - wait 5 sec.\n");
			co_await asio::steady_timer(co_await this_coro::executor, 5s).async_wait(asio::use_awaitable);
			fmt::print("[error handler] - wait cancel.\n");
		}(), asio::detached);
	}

	co_return;
}

int main()
{
	asio::io_context ctx;
	asio::co_spawn(ctx, coro(), asio::detached);
	return ctx.run();
}
