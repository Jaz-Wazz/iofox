#pragma once

// boost_asio
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/host_name_verification.hpp>
#include <boost/asio/ssl/stream_base.hpp>
#include <boost/asio/ssl/verify_mode.hpp>

// boost_beast
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>

// stl
#include <chrono>
#include <functional>
#include <stdexcept>
#include <string>

// fmt
#include <fmt/core.h>

// iofox
#include <iofox/coro.hpp>
#include <iofox/service.hpp>
#include <iofox/rethrowed.hpp>

namespace iofox::ssl
{
	inline auto context() -> iofox::coro<std::reference_wrapper<boost::asio::ssl::context>>
	{
		static iofox::service<boost::asio::ssl::context> service;
		co_return co_await service.get_or_make(boost::asio::ssl::context::tls);
	}

	inline void set_tls_extension_hostname(auto & stream, const std::string & host)
	{
		int status = SSL_set_tlsext_host_name(stream.native_handle(), host.c_str());
		if(status == SSL_TLSEXT_ERR_ALERT_FATAL)
		{
			throw std::runtime_error(fmt::format("setting tls extension error, code: {}", status));
		}
	}

	inline auto handshake_http_client
	(
		boost::beast::ssl_stream<boost::beast::tcp_stream> & stream,
		const std::string & host,
		const std::chrono::steady_clock::duration timeout = std::chrono::seconds(60)
	) -> iofox::coro<void>
	{
		iofox::ssl::set_tls_extension_hostname(stream, host);
		stream.set_verify_callback(boost::asio::ssl::host_name_verification(host));

		stream.next_layer().expires_after(timeout);
		co_await stream.async_handshake(boost::asio::ssl::stream_base::client, iofox::use_coro);
		stream.next_layer().expires_never();
	}

	inline void load_ca_certificates(auto & executor, const std::string & path)
	{
		boost::asio::co_spawn(executor, [=]() -> iofox::coro<void>
		{
			boost::asio::ssl::context & ctx = co_await iofox::ssl::context();
			ctx.load_verify_file(path);
			ctx.set_verify_mode(boost::asio::ssl::verify_peer);
		}, iofox::rethrowed);
	}
}
