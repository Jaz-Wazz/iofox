#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/execution_context.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/experimental/use_coro.hpp>
#include <boost/asio/impl/execution_context.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
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

template <typename T> class context_local
{
	prv class service_cls: public asio::execution_context::service
	{
		pbl using key_type = service;
		pbl using id = service;
		pbl T var = 0;
		pbl service_cls(asio::execution_context & context): asio::execution_context::service(context) {}
		pbl void shutdown() {}
		pbl ~service() {}
	};

	pbl auto value() -> asio::awaitable<std::reference_wrapper<T>>
	{
		auto & execution_context = (co_await this_coro::executor).context();
		if(!asio::has_service<service_cls>(execution_context)) asio::make_service<service_cls>(execution_context);
		co_return asio::use_service<service_cls>(execution_context).var;
	}
};

context_local<int> var;

auto coro(char id) -> asio::awaitable<void>
{
	int & x = co_await var.value();
	fmt::print("[coro '{}'] - Value: '{}'.\n", id, x++);
}

int main()
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
}
