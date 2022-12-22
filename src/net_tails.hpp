#pragma once
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/beast/core/basic_stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/core/noncopyable.hpp>
#include <uriparser/Uri.h>
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

namespace nt::sys
{
	// Type of async task, current asio::awaitable<T>.
	template <typename T> using coro = asio::awaitable<T>;

	// Token indicating to use async task, current asio::awaitable<T>.
	constexpr asio::use_awaitable_t<> use_coro;

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

		pbl auto get_or_make(auto... args) -> nt::sys::coro<std::reference_wrapper<T>>
		{
			auto && context = (co_await this_coro::executor).context();
			if(!asio::has_service<serv>(context)) asio::make_service<serv>(context);
			serv & s = asio::use_service<serv>(context);
			if(!s) s.emplace(std::forward<decltype(args)>(args)...);
			co_return s.value();
    	}
	};
}

namespace nt::dns
{
	// Resolve dns record from context-global service.
	inline auto resolve(std::string protocol, std::string host) -> nt::sys::coro<asio::ip::tcp::resolver::results_type>
	{
		nt::sys::service<asio::ip::tcp::resolver> service;
		auto & resolver = (co_await service.get_or_make(co_await this_coro::executor)).get();
		co_return co_await resolver.async_resolve(host, protocol, nt::sys::use_coro);
	}
}

namespace nt::ssl
{
	// Take ssl context instanse from context-global service.
	inline auto context() -> nt::sys::coro<std::reference_wrapper<asio::ssl::context>>
	{
		nt::sys::service<asio::ssl::context> service;
		co_return co_await service.get_or_make(asio::ssl::context::tlsv13_client);
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
}

namespace nt::http
{
	// Basic http client.
	class client
	{
		prv std::optional<asio::ip::tcp::socket> sock;

		pbl auto connect(std::string host) -> nt::sys::coro<void>
		{
			auto hosts = co_await nt::dns::resolve("http", host);
			sock.emplace(co_await this_coro::executor);
			co_await asio::async_connect(*sock, hosts, nt::sys::use_coro);
		}

		pbl auto write(auto & request) -> nt::sys::coro<void>
		{
			co_await beast::http::async_write(*sock, request, nt::sys::use_coro);
		}

		pbl auto read(auto & response) -> nt::sys::coro<void>
		{
			beast::flat_buffer buf;
			co_await beast::http::async_read(*sock, buf, response, nt::sys::use_coro);
		}

		pbl void disconnect()
		{
			sock->shutdown(sock->shutdown_both);
			sock->close();
		}
	};
}

namespace nt::https
{
	// Basic https client.
	class client
	{
		prv std::optional<asio::ssl::stream<asio::ip::tcp::socket>> stream;

		pbl auto connect(std::string host) -> nt::sys::coro<void>
		{
			auto hosts = co_await nt::dns::resolve("https", host);
			stream.emplace(co_await this_coro::executor, co_await nt::ssl::context());
			co_await asio::async_connect(stream->next_layer(), hosts, nt::sys::use_coro);
			nt::ssl::set_tls_extension_hostname(*stream, host);
			co_await stream->async_handshake(stream->client, nt::sys::use_coro);
		}

		pbl auto write(auto & request) -> nt::sys::coro<void>
		{
			co_await beast::http::async_write(*stream, request, nt::sys::use_coro);
		}

		pbl auto read(auto & response) -> nt::sys::coro<void>
		{
			beast::flat_buffer buf;
			co_await beast::http::async_read(*stream, buf, response, nt::sys::use_coro);
		}

		pbl auto disconnect() -> nt::sys::coro<void>
		{
			auto [err] = co_await stream->async_shutdown(asio::as_tuple(nt::sys::use_coro));
			stream->next_layer().shutdown(asio::ip::tcp::socket::shutdown_both);
			stream->next_layer().close();
			if(err != asio::ssl::error::stream_truncated) throw err;
		}
	};
}

namespace nt::sys::windows
{
	// Windows language codes.
	enum class lang: LANGID { english = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US) };

	// Change boost message language in Windows for this thread.
	inline void set_asio_message_locale(lang code)
	{
		SetThreadUILanguage(static_cast<LANGID>(code));
	}
}

namespace io
{
	// Basic url object.
	class url
	{
		pbl std::string protocol;
		pbl std::string host;
		pbl std::string path;
		pbl std::string query;
		pbl std::string fragment;

		pbl constexpr url() {}

		pbl constexpr url(const char * str): url(std::string(str)) {}

		pbl constexpr url(std::string protocol, std::string host, std::string path = "", std::string query = "", std::string fragment = "")
		: protocol(protocol), host(host), path(path), query(query), fragment(fragment) {}

		pbl constexpr url(std::string str)
		{
			// Parse url data from string.
			UriUriA uri;
			const char * error_pos;
			if(uriParseSingleUriA(&uri, str.c_str(), &error_pos) != URI_SUCCESS) throw std::runtime_error("Url parse error");

			// Initialize dynamic strings.
			protocol	= {uri.scheme.first, uri.scheme.afterLast};
			host		= {uri.hostText.first, uri.hostText.afterLast};
			query		= {uri.query.first, uri.query.afterLast};
			fragment	= {uri.fragment.first, uri.fragment.afterLast};
			if(uri.pathHead != nullptr || uri.pathTail != nullptr) path = {uri.pathHead->text.first, uri.pathTail->text.afterLast};

			// Free parser resources.
			uriFreeUriMembersA(&uri);
		}

		pbl constexpr auto serialize_location() -> std::string
		{
			return '/' + path + (query.empty() ? "" : '?' + query) + (fragment.empty() ? "" : '#' + fragment);
		}

		pbl constexpr auto serialize() -> std::string
		{
			return (protocol.empty() ? "" : protocol + "://") + host + serialize_location();
		}
	};
}

namespace io::http
{
	class client
	{
		prv using tcp_stream = asio::ip::tcp::socket;
		prv using ssl_stream = asio::ssl::stream<asio::ip::tcp::socket>;

		prv std::variant<tcp_stream, ssl_stream, std::nullopt_t> stream = std::nullopt;

		pbl auto connect(io::url url) -> nt::sys::coro<void>
		{
			if(url.protocol == "http")
			{
				stream = tcp_stream(co_await this_coro::executor);
				auto ips = co_await nt::dns::resolve("http", url.host);
				co_await asio::async_connect(std::get<tcp_stream>(stream), ips, nt::sys::use_coro);
			}
			if(url.protocol == "https")
			{
				stream = ssl_stream(co_await this_coro::executor, co_await nt::ssl::context());
				auto ips = co_await nt::dns::resolve("https", url.host);
				co_await asio::async_connect(std::get<ssl_stream>(stream).next_layer(), ips, nt::sys::use_coro);
				nt::ssl::set_tls_extension_hostname(std::get<ssl_stream>(stream), url.host);
				co_await std::get<ssl_stream>(stream).async_handshake(ssl_stream::client, nt::sys::use_coro);
			}
		}

		pbl auto write(auto & request) -> nt::sys::coro<void>
		{
			if(auto s = std::get_if<tcp_stream>(&stream)) co_await beast::http::async_write(*s, request, nt::sys::use_coro);
			if(auto s = std::get_if<ssl_stream>(&stream)) co_await beast::http::async_write(*s, request, nt::sys::use_coro);
		}

		pbl auto read(auto & response) -> nt::sys::coro<void>
		{
			beast::flat_buffer buf;
			if(auto s = std::get_if<tcp_stream>(&stream)) co_await beast::http::async_read(*s, buf, response, nt::sys::use_coro);
			if(auto s = std::get_if<ssl_stream>(&stream)) co_await beast::http::async_read(*s, buf, response, nt::sys::use_coro);
		}

		pbl auto disconnect() -> nt::sys::coro<void>
		{
			if(auto s = std::get_if<tcp_stream>(&stream))
			{
				s->shutdown(s->shutdown_both);
				s->close();
			}
			if(auto s = std::get_if<ssl_stream>(&stream))
			{
				auto [err] = co_await s->async_shutdown(asio::as_tuple(nt::sys::use_coro));
				s->next_layer().shutdown(tcp_stream::shutdown_both);
				s->next_layer().close();
				if(err != asio::ssl::error::stream_truncated) throw err;
			}
		}
	};
};

#undef asio
#undef beast
#undef this_coro
#undef pbl
#undef prv
