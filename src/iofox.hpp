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
#include <initializer_list>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <fmt/core.h>
#include <openssl/tls1.h>
#include <stdexcept>
#include <string>
#include <utility>

// iofox
#include <iofox/coro.hpp>
#include <iofox/service.hpp>
#include <iofox/rethrowed.hpp>
#include <iofox/dns.hpp>
#include <iofox/ssl.hpp>
#include <iofox/https.hpp>
#include <iofox/meta.hpp>

#define asio		boost::asio
#define beast		boost::beast
#define this_coro	asio::this_coro
#define pbl			public:
#define prv			private:

namespace io::http
{
	// Basic request object.
	template <typename T = void> class request: public beast::http::request<meta::make_body_type<T>>
	{
		prv using header_list = std::initializer_list<std::pair<std::string, std::string>>;

		pbl using beast::http::request<meta::make_body_type<T>>::operator=;
		pbl using beast::http::request<meta::make_body_type<T>>::operator[];

		pbl request(std::string method = "GET", std::string target = "/", header_list headers = {})
		{
			this->method_string(method);
			this->target(target);
			for(auto && [header, value] : headers) this->insert(header, value);
		}

		pbl template <typename X = T, typename std::enable_if<std::is_same_v<X, std::string>, int>::type = 0>
		request(std::string method, std::string target, header_list headers, const std::string & body)
		: request(std::move(method), std::move(target), std::move(headers))
		{ this->body() = body; }

		pbl template <typename X = T, typename std::enable_if<std::is_same_v<X, std::string>, int>::type = 0>
		request(std::string method, std::string target, header_list headers, const std::string && body)
		: request(std::move(method), std::move(target), std::move(headers))
		{ this->body() = std::move(body); }

		pbl template <typename X = T, typename std::enable_if<meta::vector_one_byte<X>, int>::type = 0>
		request(std::string method, std::string target, header_list headers, const std::vector<typename X::value_type> & body)
		: request(std::move(method), std::move(target), std::move(headers))
		{ this->body() = body; }

		pbl template <typename X = T, typename std::enable_if<meta::vector_one_byte<X>, int>::type = 0>
		request(std::string method, std::string target, header_list headers, const std::vector<typename X::value_type> && body)
		: request(std::move(method), std::move(target), std::move(headers))
		{ this->body() = std::move(body); }

		pbl void debug_dump()
		{
			fmt::print("[request] - {}\n", (std::stringstream() << *this).str());
		}
	};

	// Basic response object.
	template <typename T = void> class response: public beast::http::response<meta::make_body_type<T>>
	{
		prv using header_list = std::initializer_list<std::pair<std::string, std::string>>;
		pbl using beast::http::response<meta::make_body_type<T>>::operator=;
		pbl using beast::http::response<meta::make_body_type<T>>::operator[];

		pbl response(unsigned int result = 200, header_list headers = {})
		{
			this->result(result);
			for(auto && [header, value] : headers) this->insert(header, value);
		}

		pbl template <typename X = T, typename std::enable_if<std::is_same_v<X, std::string>, int>::type = 0>
		response(unsigned int result, header_list headers, const std::string & body): response(result, std::move(headers))
		{ this->body() = body; }

		pbl template <typename X = T, typename std::enable_if<std::is_same_v<X, std::string>, int>::type = 0>
		response(unsigned int result, header_list headers, std::string && body): response(result, std::move(headers))
		{ this->body() = std::move(body); }

		pbl template <typename X = T, typename std::enable_if<meta::vector_one_byte<X>, int>::type = 0>
		response(unsigned int result, header_list headers, const std::vector<typename X::value_type> & body): response(result, std::move(headers))
		{ this->body() = body; }

		pbl template <typename X = T, typename std::enable_if<meta::vector_one_byte<X>, int>::type = 0>
		response(unsigned int result, header_list headers, std::vector<typename X::value_type> && body): response(result, std::move(headers))
		{ this->body() = std::move(body); }

		pbl void check_code(int expected_code)
		{
			using base = beast::http::response<meta::make_body_type<T>>;
			if(base::result_int() != expected_code) throw std::runtime_error("unexpected_code");
		}

		pbl void check_header(const std::string_view header)
		{
			using base = beast::http::response<meta::make_body_type<T>>;
			if(base::operator[](header).empty()) throw std::runtime_error("unexpected_header");
		}

		pbl void check_contains_body()
		{
			using base = beast::http::response<meta::make_body_type<T>>;

			if(base::chunked() && !base::body().empty())
			{
				return;
			}

			if(base::has_content_length() && base::operator[]("Content-Length") != "0" && !base::body().empty())
			{
				return;
			}

			throw std::runtime_error("unexpected_empty_body");
		}

		pbl void check_not_contains_body()
		{
			using base = beast::http::response<meta::make_body_type<T>>;
			if(base::operator[]("Content-Length") != "0" || !base::body().empty())
			{
				throw std::runtime_error("unexpected_body");
			}
		}

		pbl void check_content_type(const std::string_view expected_type)
		{
			using base = beast::http::response<meta::make_body_type<T>>;
			if(base::at("Content-Type") != expected_type) throw std::runtime_error("unexpected_content_type");
		}

		pbl void debug_dump()
		{
			fmt::print("[response] - {}\n", (std::stringstream() << *this).str());
		}
	};
}

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
