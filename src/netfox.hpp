#pragma once
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/execution_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/core/noncopyable.hpp>
#include <exception>
#include <expected>
#include <fmt/core.h>
#include <functional>
#include <optional>

#define asio		boost::asio
#define this_coro	asio::this_coro
#define prv			private:
#define pbl			public:

namespace netfox::system
{
	template <typename R, typename E> class result: public std::expected<R, E>
	{
		pbl using std::expected<R, E>::expected;
		pbl R & operator~() { return this->has_value() ? this->value() : throw this->error(); }
		pbl auto error_as_unexpected() -> std::unexpected<E> { return this->error(); }
	};

	template <typename R, typename E> using async_result = asio::awaitable<result<R, E>>;

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

		pbl auto get_or_make(auto... args) noexcept -> async_result<std::reference_wrapper<T>, std::string> try
		{
			auto && context = (co_await this_coro::executor).context();
			if(!asio::has_service<serv>(context)) asio::make_service<serv>(context);
			serv & s = asio::use_service<serv>(context);
			if(!s) s.emplace(std::forward<decltype(args)>(args)...);
			co_return s.value();
		}
		catch(std::exception e)	{ co_return std::unexpected(e.what()); }
		catch(...)				{ co_return std::unexpected("unknown"); }
    };
}

namespace netfox::dns
{
	inline auto resolve(std::string protocol, std::string host) noexcept
	-> system::async_result<asio::ip::tcp::resolver::results_type, std::string>
	{
		// Decl service.
		static system::service<asio::ip::tcp::resolver> service;

		// Get service.
		auto ret = co_await service.get_or_make(co_await this_coro::executor);
		if(!ret) co_return std::unexpected(fmt::format("dns resolve error -> service error -> {}", ret.error()));

		// Resolve.
		auto [ec, result] = co_await ret.value().get().async_resolve(host, protocol, asio::as_tuple(asio::use_awaitable));
		if(ec) co_return std::unexpected(fmt::format("dns resolve error -> {}", ec.message()));
		co_return result;
	}
}

#undef asio
#undef this_coro
#undef prv
#undef pbl
