#pragma once

// boost_asio
#include <boost/asio/execution_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/verify_mode.hpp>

// stl
#include <optional>

namespace network
{
	struct dns_resolver_service: boost::asio::execution_context::service, boost::asio::ip::tcp::resolver
	{
		inline static boost::asio::execution_context::id id;
		dns_resolver_service(boost::asio::execution_context & ctx, std::optional<boost::asio::any_io_executor> executor = {})
		: boost::asio::execution_context::service(ctx), boost::asio::ip::tcp::resolver(executor.value()) {}
		void shutdown() {}
		~dns_resolver_service() {}
	};

	struct ssl_context_service: boost::asio::execution_context::service, boost::asio::ssl::context
	{
		inline static boost::asio::execution_context::id id;
		ssl_context_service(boost::asio::execution_context & ctx)
		: boost::asio::execution_context::service(ctx), boost::asio::ssl::context(boost::asio::ssl::context::tls)
		{
			load_verify_file("cacert.pem");
			set_verify_mode(boost::asio::ssl::verify_peer);
		}
		void shutdown() {}
		~ssl_context_service() {}
	};
}
