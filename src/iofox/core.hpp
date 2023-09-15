#pragma once
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/host_name_verification.hpp>
#include <boost/asio/ssl/stream_base.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/write.hpp>
#include <boost/beast/core/basic_stream.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/file.hpp>
#include <boost/beast/core/file_base.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/chunk_encode.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/serializer.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/vector_body.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/system/detail/error_code.hpp>
#include <boost/url/urls.hpp>
#include <concepts>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <ios>
#include <sstream>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <fmt/core.h>
#include <openssl/tls1.h>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <utility>
#include <winnt.h>

#define asio		boost::asio
#define beast		boost::beast
#define this_coro	asio::this_coro
#define pbl			public:
#define prv			private:

namespace io
{
	// Type of async task, current asio::awaitable<T>.
	template <typename T> using coro = asio::awaitable<T>;

	// Token indicating to use async task, current asio::awaitable<T>.
	constexpr asio::use_awaitable_t<> use_coro;

	// Token indicating to use async task and return result as tuple.
	constexpr asio::as_tuple_t<asio::use_awaitable_t<>> use_coro_tuple;

	// Token indicating to exception should be rethrown.
	constexpr class
	{
		pbl void operator ()(std::exception_ptr ptr) const
		{
			if(ptr) std::rethrow_exception(ptr);
		}
	} rethrowed;

	// Service object - make T unique for any executors. [async_local c# alternative].
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

		pbl auto get_or_make(auto... args) -> io::coro<std::reference_wrapper<T>>
		{
			auto && context = (co_await this_coro::executor).context();
			if(!asio::has_service<serv>(context)) asio::make_service<serv>(context);
			serv & s = asio::use_service<serv>(context);
			if(!s) s.emplace(std::forward<decltype(args)>(args)...);
			co_return s.value();
		}
	};
}

namespace io::dns
{
	// Resolve dns record from context-global service.
	inline auto resolve(std::string protocol, std::string host) -> io::coro<asio::ip::tcp::resolver::results_type>
	{
		io::service<asio::ip::tcp::resolver> service;
		auto & resolver = (co_await service.get_or_make(co_await this_coro::executor)).get();
		co_return co_await resolver.async_resolve(host, protocol, io::use_coro);
	}
}

namespace io::ssl
{
	// Take ssl context instanse from context-global service.
	inline auto context() -> io::coro<std::reference_wrapper<asio::ssl::context>>
	{
		io::service<asio::ssl::context> service;
		co_return co_await service.get_or_make(asio::ssl::context::tls);
	}

	// Set hostname tls extension in stream.
	constexpr void set_tls_extension_hostname(auto & stream, std::string host)
	{
		auto status = SSL_set_tlsext_host_name(stream.native_handle(), host.c_str());
		if(status == SSL_TLSEXT_ERR_ALERT_FATAL)
		{
			// [FIXME] - Use normal error handling in future.
			throw std::runtime_error(fmt::format("setting tls extension error, code: {}", status));
		}
	}

	inline auto handshake_http_client
	(
		beast::ssl_stream<beast::tcp_stream> & stream,
		const std::string & host,
		const std::chrono::steady_clock::duration timeout = std::chrono::seconds(60)
	) -> io::coro<void>
	{
		// Set host name extension.
		io::ssl::set_tls_extension_hostname(stream, host);

		// Verify certificate hostname.
		stream.set_verify_callback(asio::ssl::host_name_verification(host));

		// Handshake.
		stream.next_layer().expires_after(timeout);
		co_await stream.async_handshake(asio::ssl::stream_base::client, io::use_coro);
		stream.next_layer().expires_never();
	}

	inline void load_ca_certificates(auto & executor, std::string path)
	{
		asio::co_spawn(executor, [=]() -> io::coro<void>
		{
			asio::ssl::context & ctx = co_await io::ssl::context();
			ctx.load_verify_file(path);
			ctx.set_verify_mode(asio::ssl::verify_peer);
		}, io::rethrowed);
	}
}

namespace io::meta
{
	// Concept check type is same std::vector<T> and sizeof(T) == 1 byte.
	template <typename T>		struct is_vector					: std::false_type {};
	template <typename... T>	struct is_vector<std::vector<T...>>	: std::true_type {};
	template <typename T>		concept vector_one_byte = is_vector<T>::value && sizeof(typename T::value_type) == 1;

	// Deduse body-type from underlying object-type. [std::string -> beast::http::string_body]
	template <typename>				struct make_body_type_impl				{ using type = void;												};
	template <>						struct make_body_type_impl<void>		{ using type = beast::http::empty_body;								};
	template <>						struct make_body_type_impl<std::string>	{ using type = beast::http::string_body;							};
	template <vector_one_byte T>	struct make_body_type_impl<T>			{ using type = beast::http::vector_body<typename T::value_type>;	};
	template <typename T> using make_body_type = typename make_body_type_impl<std::remove_reference_t<T>>::type;
}

namespace io
{
	inline auto open_https_stream(const asio::any_io_executor & executor, asio::ssl::context & context, const std::string & host)
	-> io::coro<beast::ssl_stream<beast::tcp_stream>>
	{
		// Namespaces.
		using namespace std::chrono_literals;

		// Create stream.
		beast::ssl_stream<beast::tcp_stream> stream {executor, context};

		// Connect.
		stream.next_layer().expires_after(19s);
		co_await stream.next_layer().async_connect(co_await io::dns::resolve("https", host), io::use_coro);
		stream.next_layer().expires_never();

		// Handshake.
		co_await io::ssl::handshake_http_client(stream, host);

		// Return stream.
		co_return stream;
	}

	inline auto open_https_stream(const std::string & host) -> io::coro<beast::ssl_stream<beast::tcp_stream>>
	{
		co_return co_await io::open_https_stream(co_await this_coro::executor, co_await io::ssl::context(), host);
	}
}

namespace io::http
{
	class connection_dumped: public std::exception
	{
		pbl const std::string request;
		pbl const std::string response;

		pbl connection_dumped(const auto & request, const auto & response)
		: request((std::stringstream() << request).str()), response((std::stringstream() << response).str()) {}

		pbl auto what() const noexcept -> const char * override
		{
			return "connection_dumped";
		}

		pbl void save_dump(std::filesystem::path path) const
		{
			std::ofstream(path, std::ios::binary) << request << '\n' << response << '\n';
		}
	};
}

#undef asio
#undef beast
#undef this_coro
#undef pbl
#undef prv
