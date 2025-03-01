#pragma once

// boost_asio
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/deferred.hpp>

// boost_url
#include <boost/url/url_view.hpp>

// stl
#include <utility>

// local_network
#include <network/services.hpp>
#include <network/ssl_stream.hpp>

#define pbl public:
#define prt protected:
#define prv private:

namespace network
{
	class url_stream
	{
		prv enum class layer_type { tcp_socket, ssl_stream };
		pbl using executor_type = boost::asio::any_io_executor;

		prv network::ssl_stream ssl_stream;
		prv layer_type active_layer = layer_type::tcp_socket;

		pbl url_stream(boost::asio::any_io_executor executor): ssl_stream(executor) {}
		pbl auto get_executor() { return ssl_stream.get_executor(); }
		pbl auto & next_layer() { return ssl_stream; }
		pbl auto & lowest_layer() { return ssl_stream.next_layer(); }

		pbl auto async_connect(boost::urls::url_view url) -> boost::asio::awaitable<void>
		{
			if(url.scheme() != "http" && url.scheme() != "https") throw std::runtime_error("Url streams work only with http and https");
			active_layer = url.scheme() == "http" ? layer_type::tcp_socket : layer_type::ssl_stream;

			auto & dns_resolver = boost::asio::use_service<network::dns_resolver_service>(get_executor().context());
			auto endpoints = co_await dns_resolver.async_resolve(url.host(), url.scheme(), boost::asio::deferred);
			co_await boost::asio::async_connect(ssl_stream.next_layer(), endpoints, boost::asio::deferred);

			if(url.scheme() == "https")
			{
				ssl_stream.set_tlsext_hostname(url.host().c_str());
				co_await ssl_stream.async_handshake(network::ssl_stream::client, boost::asio::deferred);
			}
		}

		pbl auto async_write_some(const auto & buffers, auto && token)
		{
			if(active_layer == layer_type::tcp_socket) return lowest_layer().async_write_some(buffers, std::move(token));
			if(active_layer == layer_type::ssl_stream) return next_layer().async_write_some(buffers, std::move(token));
		}

		pbl auto async_read_some(const auto & buffers, auto && token)
		{
			if(active_layer == layer_type::tcp_socket) return lowest_layer().async_read_some(buffers, std::move(token));
			if(active_layer == layer_type::ssl_stream) return next_layer().async_read_some(buffers, std::move(token));
		}
	};
}

#undef pbl
#undef prt
#undef prv
