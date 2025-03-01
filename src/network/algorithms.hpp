#pragma once

// boost_asio
#include <boost/asio/awaitable.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/deferred.hpp>

// boost_beast
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/read.hpp>

// boost_url
#include <boost/url/url_view.hpp>

// local_http
#include <http/containers.hpp>

// local_network
#include <network/url_stream.hpp>

namespace network
{
	inline auto send(boost::urls::url_view url, http::request request) -> boost::asio::awaitable<http::response>
	{
		network::url_stream url_stream {co_await boost::asio::this_coro::executor};
		boost::beast::flat_buffer buffer;
		http::response response;

		co_await url_stream.async_connect(url);
		co_await boost::beast::http::async_write(url_stream, request, boost::asio::deferred);
		co_await boost::beast::http::async_read(url_stream, buffer, response, boost::asio::deferred);
		co_return response;
	}
}
