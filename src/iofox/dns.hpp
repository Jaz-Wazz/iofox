#pragma once

// boost_asio
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/this_coro.hpp>

// stl
#include <string_view>

// iofox
#include <iofox/coro.hpp>
#include <iofox/service.hpp>

namespace iofox::dns
{
	inline auto resolve(std::string_view protocol, std::string_view host) -> iofox::coro<boost::asio::ip::tcp::resolver::results_type>
	{
		static iofox::service<boost::asio::ip::tcp::resolver> service;
		boost::asio::ip::tcp::resolver & resolver = co_await service.get_or_make(co_await boost::asio::this_coro::executor);
		co_return co_await resolver.async_resolve(host, protocol, iofox::use_coro);
	}
}
