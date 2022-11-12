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
	template <typename T> class service: boost::noncopyable
	{
		pbl explicit service() {}

		prv class serv: public asio::execution_context::service, public std::optional<T>
		{
			pbl using key_type = serv;
			pbl using id = serv;
			pbl serv(asio::execution_context & ctx): asio::execution_context::service(ctx) {}
			pbl void shutdown() {}
		};

		pbl auto get_or_make(asio::any_io_executor && executor, auto... args) -> T &
		{
			if(!asio::has_service<serv>(executor.context())) asio::make_service<serv>(executor.context());
			serv & s = asio::use_service<serv>(executor.context());
			if(!s) s.emplace(std::forward<decltype(args)>(args)...);
			return s.value();
		}
    };
}

namespace netfox::dns
{
	auto resolve(auto protocol, auto host) -> asio::awaitable<asio::ip::tcp::resolver::results_type>
	{
		static netfox::system::service<asio::ip::tcp::resolver> service;
		auto & resolver = service.get_or_make(co_await this_coro::executor, co_await this_coro::executor);
		co_return co_await resolver.async_resolve(host, protocol, asio::use_awaitable);
	}
}

#undef asio
#undef this_coro
#undef prv
#undef pbl
