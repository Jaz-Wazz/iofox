#pragma once

// boost_core
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/core/noncopyable.hpp>

// boost_asio
#include <boost/asio/execution_context.hpp>
#include <boost/asio/this_coro.hpp>

// stl
#include <functional>
#include <optional>
#include <utility>

// iofox
#include <iofox/coro.hpp>

namespace iofox
{
	template <typename T> struct service: boost::noncopyable
	{
		explicit service() {}

		struct serv: public boost::asio::execution_context::service, public std::optional<T>
		{
			using key_type = serv;
			using id = serv;
			serv(boost::asio::execution_context & ctx): boost::asio::execution_context::service(ctx)
			{
				// boost::asio::steady_timer timer {ctx};
			}
			void shutdown() {}
		};

		auto get_or_make(auto... args) -> iofox::coro<std::reference_wrapper<T>>
		{
			auto && context = (co_await boost::asio::this_coro::executor).context();
			// if(!boost::asio::has_service<serv>(context)) boost::asio::make_service<serv>(context);
			serv & s = boost::asio::use_service<serv>(context);
			if(!s) s.emplace(std::forward<decltype(args)>(args)...);
			co_return s.value();
		}
	};
}
