#pragma once

// boost_asio
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>

namespace iofox
{
	template <typename T> using coro = boost::asio::awaitable<T>;
	constexpr boost::asio::use_awaitable_t use_coro;
	constexpr boost::asio::as_tuple_t<boost::asio::use_awaitable_t<void>> use_coro_tuple;
}
