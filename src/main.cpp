#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/execution_context.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>
#include <exception>
#include <fmt/core.h>
#include <httpfox.hpp>
#include <fastdevhax.hpp>
#include <jaja_notation.hpp>
#include <optional>

template <typename T> class context_local
{
	prv class service: public asio::execution_context::service, public std::optional<T>
	{
		pbl using key_type = service;
		pbl using id = service;
		pbl service(asio::execution_context & ctx): asio::execution_context::service(ctx) {}
		pbl void shutdown() {}
	};

	pbl auto value_or_emplace(asio::any_io_executor && executor, auto... args) -> T &
	{
		if(!asio::has_service<service>(executor.context())) asio::make_service<service>(executor.context());
		service & s = asio::use_service<service>(executor.context());
		if(!s) s.emplace(std::forward<decltype(args)>(args)...);
		return s.value();
	}
};

class res
{
	pbl res() { fmt::print("construct.\n"); }
	pbl ~res() { fmt::print("destruct.\n"); }
	pbl int i = 10;
};

auto coro(char id) -> asio::awaitable<void>
{
	static context_local<res> var;
	auto & x = var.value_or_emplace(co_await this_coro::executor);
	fmt::print("[coro '{}'] - Value: '{}'.\n", id, x.i++);
}

int main() try
{
	// First ctx.
	{
		asio::io_context ctx;
		for(int i = 0; i < 10; i++) asio::co_spawn(ctx, coro('a'), hf::rethrowed);
		ctx.run();
	}

	// Second ctx.
	{
		asio::io_context ctx;
		for(int i = 0; i < 10; i++) asio::co_spawn(ctx, coro('b'), hf::rethrowed);
		ctx.run();
	}
	return 0;
} catch(std::exception & e) { fmt::print("Except: '{}'.\n", e.what()); }
