#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/execution_context.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <exception>
#include <fmt/core.h>
#include <httpfox.hpp>
#include <fastdevhax.hpp>
#include <jaja_notation.hpp>
#include <optional>
#include <boost/core/noncopyable.hpp>

namespace netfox::system
{
	template <typename T> class context_local: boost::noncopyable
	{
		pbl explicit context_local() {}

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
}

namespace netfox::dns
{
	auto resolve(auto protocol, auto host) -> asio::awaitable<asio::ip::tcp::resolver::results_type>
	{
		static netfox::system::context_local<asio::ip::tcp::resolver> resolver;
		auto & r = resolver.value_or_emplace(co_await this_coro::executor, co_await this_coro::executor);
		co_return co_await r.async_resolve(host, protocol, asio::use_awaitable);
	}
}

auto coro(char id) -> asio::awaitable<void>
{
	// netfox::dns::interrupt();
	// netfox::ssl::context()

	auto result = co_await netfox::dns::resolve("https", "google.com");
	for(auto el : result) fmt::print("[{}] - Ip: '{}'.\n", id, el.endpoint().address().to_string());

	// static netfox::system::context_local<int> var;
	// fmt::print("[coro '{}'] - Value: '{}'.\n", id, var.value_or_emplace(co_await this_coro::executor, 25)++);
}

int main() try
{
	// First ctx.
	{
		asio::io_context ctx;
		for(int i = 0; i < 2; i++) asio::co_spawn(ctx, coro('a'), hf::rethrowed);
		ctx.run();
	}

	// Second ctx.
	{
		asio::io_context ctx;
		for(int i = 0; i < 2; i++) asio::co_spawn(ctx, coro('b'), hf::rethrowed);
		ctx.run();
	}
	return 0;
} catch(std::exception & e) { fmt::print("Except: '{}'.\n", e.what()); }
