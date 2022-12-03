#pragma once
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/outcome/outcome.hpp>
#include <boost/outcome/try.hpp>
#include <as_result.hpp>
#include <boost/outcome/success_failure.hpp>
#include <exception>
#include <optional>
#include <string>
#include <utility>

#define asio		boost::asio
#define this_coro	asio::this_coro
#define oc			BOOST_OUTCOME_V2_NAMESPACE
#define pbl			public:
#define prv			private:
#define nt_check(x)	if(!x) co_return x.as_failure();
#define nt_try(x)	BOOST_OUTCOME_CO_TRYX(x)
#define nt_async(x)	BOOST_OUTCOME_CO_TRYX(co_await x)

namespace nt
{
	template <typename T> using result = asio::awaitable<oc::outcome<T>>;
	constexpr auto success(auto... args) { return oc::success(std::forward<decltype(args)>(args)...); }

	inline auto use_result() { return as_result(asio::use_awaitable); }

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

		pbl auto get_or_make(auto... args) -> nt::result<std::reference_wrapper<T>> try
		{
			auto && context = (co_await this_coro::executor).context();
			if(!asio::has_service<serv>(context)) asio::make_service<serv>(context);
			serv & s = asio::use_service<serv>(context);
			if(!s) s.emplace(std::forward<decltype(args)>(args)...);
			co_return s.value();
		} catch(...) { co_return std::current_exception(); }
    };
}

namespace nt::dns
{
	inline auto resolve(std::string protocol, std::string host) -> nt::result<asio::ip::tcp::resolver::results_type>
	{
		nt::service<asio::ip::tcp::resolver> service;
		auto & resolver = nt_async(service.get_or_make(co_await this_coro::executor)).get();
		co_return nt_async(resolver.async_resolve(host, protocol, nt::use_result()));
	}
}

#undef asio
#undef this_coro
#undef oc
#undef pbl
#undef prv
