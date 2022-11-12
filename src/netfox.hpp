#pragma once
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/execution_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/core/noncopyable.hpp>
#include <optional>

#define asio		boost::asio
#define this_coro	asio::this_coro
#define prv			private:
#define pbl			public:

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

#undef asio
#undef this_coro
#undef prv
#undef pbl
