#pragma once

// boost_asio
#include <boost/asio/execution_context.hpp>
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ip/tcp.hpp>

// boost_system
#include <boost/system/system_error.hpp>
#include <boost/system/error_code.hpp>

// openssl
#include <openssl/tls1.h>

// local_network
#include <network/services.hpp>

namespace network
{
	struct ssl_stream: boost::asio::ssl::stream<boost::asio::ip::tcp::socket>
	{
		ssl_stream(boost::asio::any_io_executor executor)
		: boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(executor, boost::asio::use_service<network::ssl_context_service>(executor.context())) {}

		void set_tlsext_hostname(const char * hostname)
		{
			auto ret = SSL_set_tlsext_host_name(native_handle(), hostname);
			if(ret == SSL_TLSEXT_ERR_ALERT_FATAL) throw boost::system::system_error(boost::system::error_code(boost::asio::ssl::error::unexpected_result));
		}
	};
}
