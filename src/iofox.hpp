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
#include <boost/system/error_code.hpp>
#include <boost/url/urls.hpp>
#include <string_view>
#include <fmt/core.h>
#include <openssl/tls1.h>
#include <stdexcept>
#include <string>

// iofox
#include <iofox/coro.hpp>
#include <iofox/service.hpp>
#include <iofox/rethrowed.hpp>
#include <iofox/dns.hpp>
#include <iofox/ssl.hpp>
#include <iofox/https.hpp>
#include <iofox/meta.hpp>
#include <iofox/http.hpp>

#define asio		boost::asio
#define beast		boost::beast
#define this_coro	asio::this_coro
#define pbl			public:
#define prv			private:

namespace io::http
{
	template <typename T = std::string>
	inline auto send
	(
		beast::ssl_stream<beast::tcp_stream> & stream,
		const auto & request,
		const std::chrono::steady_clock::duration timeout = std::chrono::seconds(60)
	)
	-> io::coro<io::http::response<T>>
	{
		stream.next_layer().expires_after(timeout);
		co_await beast::http::async_write(stream, request, io::use_coro);
		io::http::response<T> response;
		beast::flat_buffer buffer;
		co_await beast::http::async_read(stream, buffer, response, io::use_coro);
		stream.next_layer().expires_never();

		co_return response;
	}

	template <typename T = std::string>
	inline auto send
	(
		const boost::url url,
		const auto & request,
		const std::chrono::steady_clock::duration timeout = std::chrono::seconds(60)
	)
	-> io::coro<io::http::response<T>>
	{
		if(url.scheme() != "https") throw std::runtime_error("unsupported_protocol");
		auto stream = co_await io::open_https_stream(url.host());
		co_return co_await io::http::send<T>(stream, request, timeout);
	}

	template <typename T = std::string>
	auto send
	(
		const std::string_view url,
		const auto & request,
		const std::chrono::steady_clock::duration timeout = std::chrono::seconds(60)
	)
	-> io::coro<io::http::response<T>>
	{
		co_return co_await io::http::send<T>(boost::url(url), request, timeout);
	}

	extern template auto send<std::string>
	(
		const std::string_view url,
		const io::http::request<void> & request,
		const std::chrono::steady_clock::duration timeout
	)
	-> io::coro<io::http::response<std::string>>;
}

#undef asio
#undef beast
#undef this_coro
#undef pbl
#undef prv
