#pragma once
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/ssl/error.hpp>
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
#include <boost/beast/core/flat_buffer.hpp>
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
#include <boost/core/noncopyable.hpp>
#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <system_error>
#include <type_traits>
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

namespace io
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

		pbl auto get_or_make(auto... args) -> io::coro<std::reference_wrapper<T>>
		{
			auto && context = (co_await this_coro::executor).context();
			if(!asio::has_service<serv>(context)) asio::make_service<serv>(context);
			serv & s = asio::use_service<serv>(context);
			if(!s) s.emplace(std::forward<decltype(args)>(args)...);
			co_return s.value();
    	}
	};

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

	// Basic file object.
	class file: public beast::file
	{
		pbl file() = default;

		pbl void operator=(beast::file && other)
		{
			beast::file::operator=(std::move(other));
		}

		pbl file(const char * path, beast::file_mode mode = beast::file_mode::write)
		{
			beast::error_code e;
			open(path, mode, e);
			if(e) throw std::system_error(e);
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
}

namespace io::windows
{
	// Windows language codes.
	enum class lang: LANGID { english = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US) };

	// Change boost language in Windows for this thread.
	inline void set_asio_locale(lang code)
	{
		SetThreadUILanguage(static_cast<LANGID>(code));
	}
}

namespace io::meta
{
	// Concept check type for std::nullopt.
	template <typename T> concept not_nullopt = typeid(T) != typeid(std::nullopt);

	// Concept check type for string body.
	template <typename T> concept is_string_body = typeid(T) == typeid(beast::http::string_body);

	// Concept check type for file body.
	template <typename T> concept is_file_body = typeid(T) == typeid(beast::http::file_body);

	// Concept check type for not sameless.
	template <typename T, typename... X> concept not_same = (typeid(T) != typeid(X) && ...);

	// Concept check type is same std::vector<T> and sizeof(T) == 1 byte.
	template <typename T> concept vector_one_byte = typeid(std::vector<typename T::value_type>) == typeid(T) && sizeof(T::value_type) == 1;

	// Concept check type is io::file or beast::file.
	template <typename T> concept any_file_type = typeid(T) == typeid(io::file) || typeid(T) == typeid(beast::file);

	// Deduse body-type from underlying object-type. [std::string -> beast::http::string_body]
	template <typename>				struct make_body_type_impl;
	template <>						struct make_body_type_impl<void>		{ using type = beast::http::empty_body;								};
	template <>						struct make_body_type_impl<std::string>	{ using type = beast::http::string_body;							};
	template <vector_one_byte T>	struct make_body_type_impl<T>			{ using type = beast::http::vector_body<typename T::value_type>;	};
	template <any_file_type T>		struct make_body_type_impl<T>			{ using type = beast::http::file_body;								};
	template <typename T> using make_body_type = typename make_body_type_impl<std::remove_reference_t<T>>::type;

	// Fast overloaded object creator for "std::variant" unpack.
	template <class... Ts> struct overloaded: Ts... { using Ts::operator()...; };
	template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
}

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
		{ this->body() = body; this->prepare_payload(); }

		pbl template <typename X = T, typename std::enable_if<std::is_same_v<X, std::string>, int>::type = 0>
		request(std::string method, std::string target, header_list headers, const std::string && body)
		: request(std::move(method), std::move(target), std::move(headers))
		{ this->body() = std::move(body); this->prepare_payload(); }

		pbl template <typename X = T, typename std::enable_if<meta::vector_one_byte<X>, int>::type = 0>
		request(std::string method, std::string target, header_list headers, const std::vector<typename X::value_type> & body)
		: request(std::move(method), std::move(target), std::move(headers))
		{ this->body() = body; this->prepare_payload(); }

		pbl template <typename X = T, typename std::enable_if<meta::vector_one_byte<X>, int>::type = 0>
		request(std::string method, std::string target, header_list headers, const std::vector<typename X::value_type> && body)
		: request(std::move(method), std::move(target), std::move(headers))
		{ this->body() = std::move(body); this->prepare_payload(); }

		pbl template <typename X = T, typename std::enable_if<std::is_same_v<X, io::file>, int>::type = 0>
		request(std::string method, std::string target, header_list headers, io::file && body)
		: request(std::move(method), std::move(target), std::move(headers))
		{ this->body().file() = std::move(body); this->prepare_payload(); }

		pbl template <typename X = T, typename std::enable_if<std::is_same_v<X, beast::file>, int>::type = 0>
		request(std::string method, std::string target, header_list headers, beast::file && body)
		: request(std::move(method), std::move(target), std::move(headers))
		{ this->body().file() = std::move(body); this->prepare_payload(); }
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
		{ this->body() = body; this->prepare_payload(); }

		pbl template <typename X = T, typename std::enable_if<std::is_same_v<X, std::string>, int>::type = 0>
		response(unsigned int result, header_list headers, std::string && body): response(result, std::move(headers))
		{ this->body() = std::move(body); this->prepare_payload(); }

		pbl template <typename X = T, typename std::enable_if<meta::vector_one_byte<X>, int>::type = 0>
		response(unsigned int result, header_list headers, const std::vector<typename X::value_type> & body): response(result, std::move(headers))
		{ this->body() = body; this->prepare_payload(); }

		pbl template <typename X = T, typename std::enable_if<meta::vector_one_byte<X>, int>::type = 0>
		response(unsigned int result, header_list headers, std::vector<typename X::value_type> && body): response(result, std::move(headers))
		{ this->body() = std::move(body); this->prepare_payload(); }

		pbl template <typename X = T, typename std::enable_if<std::is_same_v<X, io::file>, int>::type = 0>
		response(unsigned int result, header_list headers, io::file && body): response(result, std::move(headers))
		{ this->body().file() = std::move(body); this->prepare_payload(); }

		pbl template <typename X = T, typename std::enable_if<std::is_same_v<X, beast::file>, int>::type = 0>
		response(unsigned int result, header_list headers, beast::file && body): response(result, std::move(headers))
		{ this->body().file() = std::move(body); this->prepare_payload(); }
	};

	// Basic request header object.
	class request_header: public beast::http::request_header<>
	{
		prv using base = beast::http::request_header<>;
		prv using header_list = std::initializer_list<std::pair<std::string, std::string>>;

		pbl using base::operator=;
		pbl using base::operator[];

		pbl request_header(std::string method = "GET", std::string target = "/", header_list headers = {})
		{
			this->method_string(method);
			this->target(target);
			for(auto && [header, value] : headers) this->insert(header, value);
		}
	};

	// Basic response header object.
	class response_header: public beast::http::response_header<>
	{
		prv using base = beast::http::response_header<>;
		prv using header_list = std::initializer_list<std::pair<std::string, std::string>>;

		pbl using base::operator=;
		pbl using base::operator[];

		pbl response_header(unsigned int result = 200, header_list headers = {})
		{
			this->result(result);
			for(auto && [header, value] : headers) this->insert(header, value);
		}
	};

	// Basic high-level http/https client.
	class client
	{
		prv using tcp_stream	= asio::ip::tcp::socket;
		prv using ssl_stream	= asio::ssl::stream<asio::ip::tcp::socket>;
		prv using empty_body	= beast::http::empty_body;
		prv using string_body	= beast::http::string_body;
		prv using buffer_body	= beast::http::buffer_body;
		prv using file_body		= beast::http::file_body;

		prv template <typename T> using vector_body	= beast::http::vector_body<T>;
		prv using any_stream = std::variant<std::nullopt_t, tcp_stream, ssl_stream>;

		prv template <typename T> class response_parser: public beast::http::response_parser<T>
		{
			using beast::http::response_parser<T>::response_parser;
			pbl auto message() -> beast::http::response<T> & { return this->get(); }
		};

		prv template <typename T> class request_serializer: public beast::http::request_serializer<T>
		{
			prv beast::http::request<T> msg;
			pbl request_serializer(auto... args): msg(std::forward<decltype(args)>(args)...), beast::http::request_serializer<T>(msg) {}
			pbl auto message() -> beast::http::request<T> & { return msg; }
		};

		prv any_stream stream = std::nullopt;
		prv std::optional<response_parser<buffer_body>> parser;
		prv std::optional<request_serializer<buffer_body>> serializer;
		prv std::optional<beast::flat_buffer> buf;

		pbl auto connect(io::url url) -> io::coro<void>
		{
			if(url.protocol == "http")
			{
				tcp_stream stream {co_await this_coro::executor};
				auto ips = co_await io::dns::resolve("http", url.host);
				co_await asio::async_connect(stream, ips, io::use_coro);
				this->stream = std::move(stream);
			}
			if(url.protocol == "https")
			{
				ssl_stream stream {co_await this_coro::executor, co_await io::ssl::context()};
				auto ips = co_await io::dns::resolve("https", url.host);
				co_await asio::async_connect(stream.next_layer(), ips, io::use_coro);
				io::ssl::set_tls_extension_hostname(stream, url.host);
				co_await stream.async_handshake(stream.client, io::use_coro);
				this->stream = std::move(stream);
			}
		}

		pbl auto write_header(auto & request_header) -> io::coro<void>
		{
			// Initialize.
			serializer.emplace(request_header);

			// Write header.
			co_await std::visit(meta::overloaded
			{
				[&](auto && stream) -> io::coro<void> { co_await beast::http::async_write_header(stream, *serializer, io::use_coro); },
				[](std::nullopt_t) -> io::coro<void> { co_return; }
			}, stream);
		}

		pbl auto write_body_octets(const char * buffer, std::size_t size, bool last_buffer = false) -> io::coro<std::size_t>
		{
			co_return co_await std::visit(meta::overloaded
			{
				[&](auto && stream) -> io::coro<std::size_t>
				{
					serializer->message().body().data = const_cast<char *>(buffer);
					serializer->message().body().size = size;
					serializer->message().body().more = !last_buffer;
					auto [err, bytes_writed] = co_await beast::http::async_write(stream, *serializer, asio::as_tuple(io::use_coro));
					if(err.failed() && err != beast::http::error::need_buffer) throw std::system_error(err);
					co_return bytes_writed;
				},
				[](std::nullopt_t) -> io::coro<std::size_t> { co_return 0; },
			}, stream);
		}

		pbl auto write_body_chunk_tail() -> io::coro<void>
		{
			co_await std::visit(meta::overloaded
			{
				[&](auto && stream) -> io::coro<void> { co_await asio::async_write(stream, beast::http::make_chunk_last(), io::use_coro); },
				[](std::nullopt_t) -> io::coro<void> { co_return; }
			}, stream);
		}

		pbl auto read_header(auto & response_header) -> io::coro<void>
		{
			// Initialize.
			parser.emplace();
			parser->body_limit(boost::none);
			buf.emplace();

			// Read header.
			co_await std::visit(meta::overloaded
			{
				[&](auto && stream) -> io::coro<void> { co_await beast::http::async_read_header(stream, *buf, *parser, io::use_coro); },
				[](std::nullopt_t) -> io::coro<void> { co_return; }
			}, stream);

			// Return headers.
			response_header = parser->get().base();
		}

		pbl auto read_body_octets(char * buffer, std::size_t size) -> io::coro<std::optional<std::size_t>>
		{
			// Check parser end state -> Set-up buffers -> Perform read -> Return readed chunk size.
			co_return co_await std::visit(meta::overloaded
			{
				[&](auto && stream) -> io::coro<std::optional<std::size_t>>
				{
					if(!parser->is_done())
					{
						parser->get().body().data = buffer;
						parser->get().body().size = size;
						auto [err, bytes_readed] = co_await beast::http::async_read(stream, *buf, *parser, asio::as_tuple(io::use_coro));
						if(err.failed() && err != beast::http::error::need_buffer) throw std::system_error(err);
						co_return size - parser->get().body().size;
					} else co_return std::nullopt;
				},
				[](std::nullopt_t) -> io::coro<std::optional<std::size_t>> { co_return std::nullopt; },
			}, stream);
		}

		pbl auto read_body(std::string & body) -> io::coro<void>
		{
			if(parser->message().has_content_length())
			{
				std::size_t content_length = std::stoull(parser->message()["Content-Length"].to_string());
				body.resize_and_overwrite(content_length, [&](auto...) { return content_length; });
				auto octets_readed = co_await read_body_octets(body.data(), content_length);
				body.resize(octets_readed.value_or(0));
			}
			if(parser->message().chunked())
			{
				for(int i = 0; true; i += 1024)
				{
					body.resize_and_overwrite(i + 1024, [&](auto...) { return i + 1024; });
					auto octets_readed = co_await read_body_octets(body.data() + i, 1024);
					if(octets_readed.value_or(0) < 1024) { body.resize(i + octets_readed.value_or(0)); break; }
				}
			}
		}

		pbl auto read(io::http::response<std::string> & response) -> io::coro<void>
		{
			co_await read_header(response.base());
			co_await read_body(response.body());
		}

		pbl auto write_body(auto & body) -> io::coro<void>
		{
			co_return;
		}

		pbl void disconnect()
		{
			std::visit(meta::overloaded
			{
				[](tcp_stream & stream) { stream.shutdown(stream.shutdown_both); stream.close(); },
				[](ssl_stream & stream) { stream.next_layer().shutdown(tcp_stream::shutdown_both); stream.next_layer().close(); },
				[](std::nullopt_t) {},
			}, stream);
		}
	};
};

#undef asio
#undef beast
#undef this_coro
#undef pbl
#undef prv
