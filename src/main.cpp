#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <fmt/core.h>
#include <httpfox.hpp>
#include <fastdevhax.hpp>
#include <jaja_notation.hpp>

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
