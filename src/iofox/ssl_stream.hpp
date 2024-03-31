#pragma once

// boost_asio
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>

// boost_system
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

// openssl
#include <openssl/tls1.h>

// stl
#include <system_error>
#include <utility>

namespace iofox
{
	template <class layer_type = boost::asio::ip::tcp::socket>
	struct ssl_stream: public boost::asio::ssl::stream<layer_type>
	{
		ssl_stream(auto && arg, boost::asio::ssl::context & ssl_context)
		: boost::asio::ssl::stream<layer_type>(std::forward<decltype(arg)>(arg), ssl_context) {}

		ssl_stream(auto && arg, boost::asio::ssl::stream<layer_type>::native_handle_type handle)
		: boost::asio::ssl::stream<layer_type>(std::forward<decltype(arg)>(arg), handle) {}

		ssl_stream(boost::asio::ssl::stream<layer_type> && other)
		: boost::asio::ssl::stream<layer_type>(std::move(other)) {}

		ssl_stream & operator=(boost::asio::ssl::stream<layer_type> && other)
		{
			boost::asio::ssl::stream<layer_type>::operator=(std::move(other));
			return *this;
		}

		void set_tlsext_hostname(const char * hostname, boost::system::error_code & ec)
		{
			auto ret = SSL_set_tlsext_host_name(this->native_handle(), hostname);
			if(ret == SSL_TLSEXT_ERR_ALERT_FATAL) ec = boost::asio::ssl::error::unexpected_result;
		}

		void set_tlsext_hostname(const char * hostname)
		{
			boost::system::error_code ec;
			set_tlsext_hostname(hostname, ec);
			if(ec.failed()) throw std::system_error(ec);
		}
	};
}
