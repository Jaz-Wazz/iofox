#pragma once
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/connect.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/core/noncopyable.hpp>
#include <optional>
#include <string>
#include <utility>
#include <winnt.h>

#define asio		boost::asio
#define beast		boost::beast
#define this_coro	asio::this_coro
#define pbl			public:
#define prv			private:

namespace nt::sys
{
	template <typename T> using coro = asio::awaitable<T>;
	constexpr asio::use_awaitable_t<> use_coro;
	constexpr class
	{
		pbl void operator ()(std::exception_ptr ptr) const
		{
			if(ptr) std::rethrow_exception(ptr);
		}
	} rethrowed;

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
	inline auto resolve(std::string protocol, std::string host) -> nt::sys::coro<asio::ip::tcp::resolver::results_type>
	{
		nt::sys::service<asio::ip::tcp::resolver> service;
		auto & resolver = (co_await service.get_or_make(co_await this_coro::executor)).get();
		co_return co_await resolver.async_resolve(host, protocol, nt::sys::use_coro);
	}
}

namespace nt::ssl
{
	inline auto context() -> nt::sys::coro<std::reference_wrapper<asio::ssl::context>>
	{
		nt::sys::service<asio::ssl::context> service;
		co_return co_await service.get_or_make(asio::ssl::context::tlsv13_client);
	}
}

namespace nt::http
{
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

#undef asio
#undef beast
#undef this_coro
#undef pbl
#undef prv
