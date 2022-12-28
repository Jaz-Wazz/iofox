#pragma once
#include "iofox.hpp"
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
#include <boost/beast/core/file.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/core/noncopyable.hpp>
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
#include <iostream>

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

namespace io::http
{
	// Basic high-level http/https client.
	class client
	{
		prv using tcp_stream = asio::ip::tcp::socket;
		prv using ssl_stream = asio::ssl::stream<asio::ip::tcp::socket>;
		prv using parser_buffer = beast::http::response_parser<beast::http::buffer_body>;
		prv using parser_string = beast::http::response_parser<beast::http::string_body>;
		prv using parser_empty = beast::http::response_parser<beast::http::empty_body>;
		prv using parser_file = beast::http::response_parser<beast::http::file_body>;

		prv std::variant<tcp_stream, ssl_stream, std::nullopt_t> stream = std::nullopt;
		prv std::variant<parser_buffer, parser_string, parser_empty, parser_file, std::nullopt_t> parser = std::nullopt;
		prv std::optional<beast::flat_buffer> buf;

		pbl auto connect(io::url url) -> io::coro<void>
		{
			if(url.protocol == "http")
			{
				stream = tcp_stream(co_await this_coro::executor);
				auto ips = co_await io::dns::resolve("http", url.host);
				co_await asio::async_connect(std::get<tcp_stream>(stream), ips, io::use_coro);
			}
			if(url.protocol == "https")
			{
				stream = ssl_stream(co_await this_coro::executor, co_await io::ssl::context());
				auto ips = co_await io::dns::resolve("https", url.host);
				co_await asio::async_connect(std::get<ssl_stream>(stream).next_layer(), ips, io::use_coro);
				io::ssl::set_tls_extension_hostname(std::get<ssl_stream>(stream), url.host);
				co_await std::get<ssl_stream>(stream).async_handshake(ssl_stream::client, io::use_coro);
			}
		}

		pbl auto write(auto & request) -> io::coro<void>
		{
			if(auto s = std::get_if<tcp_stream>(&stream)) co_await beast::http::async_write(*s, request, io::use_coro);
			if(auto s = std::get_if<ssl_stream>(&stream)) co_await beast::http::async_write(*s, request, io::use_coro);
		}

		pbl auto read(auto & response) -> io::coro<void>
		{
			// Create buf, if not exists.
			if(!buf) buf.emplace();

			// Deduse body type from user response.
			using body_type = typename std::remove_reference_t<decltype(response)>::body_type;

			// Deduse parser type with user body type.
			using parser_type = beast::http::response_parser<body_type>;

			// Create dedused parser in pre-allocated variant -> Initialize from moved user response.
			parser.emplace<parser_type>(std::move(response));

			co_await std::visit([&](auto && stream, auto && parser) -> io::coro<void>
			{
				// Check if [stream not nullopt] and [parser not nullopt].
				if constexpr (typeid(decltype(stream)) != typeid(std::nullopt_t) && typeid(decltype(parser)) != typeid(std::nullopt_t))
				{
					// Deduse body type for used parser.
					using parser_body_type = typename std::remove_reference_t<decltype(parser)>::value_type::body_type;

					// Perform read.
					co_await beast::http::async_read(stream, *buf, parser, io::use_coro);

					// Check if [parser body type] == [response body type] -> Move response back to user.
					if constexpr (typeid(parser_body_type) == typeid(body_type)) response = std::move(parser.get());
				}
			}, stream, parser);

			// Destroy parser.
			parser = std::nullopt;
		}

		pbl auto read_header(auto & response_header) -> io::coro<void>
		{
			// Create buf, if not exists.
			if(!buf) buf.emplace();

			// Create parser with empty body in pre-allocated variant -> Initialize from moved user response headers.
			parser.emplace<parser_empty>(std::move(response_header));

			co_await std::visit([&](auto && stream, auto && parser) -> io::coro<void>
			{
				// Check if [stream not nullopt] and [parser not nullopt].
				if constexpr (typeid(decltype(stream)) != typeid(std::nullopt_t) && typeid(decltype(parser)) != typeid(std::nullopt_t))
				{
					// Perform read header.
					co_await beast::http::async_read_header(stream, *buf, parser, io::use_coro);

					// Move response headers back to user.
					response_header = parser.get();
				}
			}, stream, parser);
		}

		prv template <typename T> constexpr void change_parser_body()
		{
			std::visit([&](auto && stream, auto && parser)
			{
				using parser_type = std::remove_reference_t<decltype(parser)>;
				using parser_type_new = beast::http::response_parser<T>;

				if constexpr (typeid(parser_type) != typeid(parser_type_new) && typeid(parser_type) != typeid(std::nullopt))
				{
					// Extract parser from variant.
					parser_type_new p {std::move(parser)};

					// Put parser to variant.
					this->parser.emplace<parser_type_new>(std::move(parser_type(std::move(p))));
				}
			}, stream, parser);
		}

		pbl auto read_body(auto & body) -> io::coro<void>
		{
			// Create buf, if not exists.
			if(!buf) buf.emplace();

			if constexpr (typeid(body) == typeid(std::string))
			{
				change_parser_body<beast::http::string_body>();
				std::get<parser_string>(parser).get().body() = std::move(body);
			}
			if constexpr (typeid(body) == typeid(beast::http::file_body::value_type))
			{
				change_parser_body<beast::http::file_body>();
				std::get<parser_file>(parser).get().body() = std::move(body);
			}
			if constexpr (typeid(body) == typeid(beast::file))
			{
				change_parser_body<beast::http::file_body>();
				std::get<parser_file>(parser).get().body().file() = std::move(body);
			}

			co_await std::visit([&](auto && stream, auto && parser) -> io::coro<void>
			{
				// Check if [stream not nullopt] and [parser not nullopt].
				if constexpr (typeid(decltype(stream)) != typeid(std::nullopt_t) && typeid(decltype(parser)) != typeid(std::nullopt_t))
				{
					// Perform read body.
					co_await beast::http::async_read(stream, *buf, parser, io::use_coro);

					// Deduse body type for used parser.
					using parser_body_type_value = typename std::remove_reference_t<decltype(parser)>::value_type::body_type::value_type;

					if constexpr (typeid(parser_body_type_value) == typeid(body) && typeid(body) == typeid(beast::http::file_body::value_type))
					{
						fmt::print("file.\n");
						body = std::move(parser.get().body());
						co_return;
					}

					if constexpr (typeid(parser_body_type_value) == typeid(beast::http::file_body::value_type) && typeid(body) == typeid(beast::file))
					{
						fmt::print("file 2.\n");
						body = std::move(parser.get().body().file());
						co_return;
					}

					// Return body.
					if constexpr (typeid(parser_body_type_value) == typeid(body))
					{
						fmt::print("non-file.\n");
						body = std::move(parser.get().body());
						co_return;
					}
				}
			}, stream, parser);
		}

		pbl auto read_body(char * buffer, std::size_t size) -> io::coro<std::optional<std::size_t>>
		{
			// if(!buf) buf.emplace();
			// if(!parser) parser.emplace();
			// if(!parser->is_done())
			// {
			// 	parser->get().body().data = buffer;
			// 	parser->get().body().size = size;
			// 	if(auto s = std::get_if<tcp_stream>(&stream))
			// 	{
			// 		auto [err, bytes_readed] = co_await beast::http::async_read(*s, *buf, *parser, asio::as_tuple(io::use_coro));
			// 		if(err.failed() && err != beast::http::error::need_buffer) throw std::system_error(err);
			// 	}
			// 	if(auto s = std::get_if<ssl_stream>(&stream))
			// 	{
			// 		auto [err, bytes_readed] = co_await beast::http::async_read(*s, *buf, *parser, asio::as_tuple(io::use_coro));
			// 		if(err.failed() && err != beast::http::error::need_buffer) throw std::system_error(err);
			// 	}
			// 	co_return size - parser->get().body().size;
			// } else co_return std::nullopt;
			co_return 0;
		}

		pbl void disconnect()
		{
			if(auto s = std::get_if<tcp_stream>(&stream))
			{
				s->shutdown(s->shutdown_both);
				s->close();
			}
			if(auto s = std::get_if<ssl_stream>(&stream))
			{
				s->next_layer().shutdown(tcp_stream::shutdown_both);
				s->next_layer().close();
			}
		}
	};

	// Basic request object.
	template <typename T = void> class request;

	// Basic request object without body.
	template <> class request<void>: public beast::http::request<beast::http::empty_body>
	{
		prv using base = beast::http::request<beast::http::empty_body>;
		prv using header_list = std::initializer_list<std::pair<std::string, std::string>>;

		pbl using base::operator=;
		pbl using base::operator[];

		pbl request(std::string method = "GET", std::string target = "/", header_list headers = {})
		{
			this->method_string(method);
			this->target(target);
			for(auto && [header, value] : headers) this->insert(header, value);
		}
	};

	// Basic request object with string body.
	template <> class request<std::string>: public beast::http::request<beast::http::string_body>
	{
		prv using base = beast::http::request<beast::http::string_body>;
		prv using header_list = std::initializer_list<std::pair<std::string, std::string>>;

		pbl using base::operator=;
		pbl using base::operator[];

		pbl request(std::string method = "GET", std::string target = "/", header_list headers = {})
		{
			this->method_string(method);
			this->target(target);
			for(auto && [header, value] : headers) this->insert(header, value);
		}

		pbl request(std::string method, std::string target, header_list headers, const std::string & body)
		{
			this->method_string(method);
			this->target(target);
			for(auto && [header, value] : headers) this->insert(header, value);
			this->body() = body;
		}
	};

	// Basic response object.
	template <typename T = void> class response;

	// Basic response object without body.
	template <> class response<void>: public beast::http::response<beast::http::empty_body>
	{
		prv using base = beast::http::response<beast::http::empty_body>;
		prv using header_list = std::initializer_list<std::pair<std::string, std::string>>;

		pbl using base::operator=;
		pbl using base::operator[];

		pbl response(unsigned int result = 200, header_list headers = {})
		{
			this->result(result);
			for(auto && [header, value] : headers) this->insert(header, value);
		}
	};

	// Basic response object with string body.
	template <> class response<std::string>: public beast::http::response<beast::http::string_body>
	{
		prv using base = beast::http::response<beast::http::string_body>;
		prv using header_list = std::initializer_list<std::pair<std::string, std::string>>;

		pbl using base::operator=;
		pbl using base::operator[];

		pbl response(unsigned int result = 200, header_list headers = {})
		{
			this->result(result);
			for(auto && [header, value] : headers) this->insert(header, value);
		}

		pbl response(unsigned int result, header_list headers, const std::string & body)
		{
			this->result(result);
			for(auto && [header, value] : headers) this->insert(header, value);
			this->body() = body;
		}
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
};

#undef asio
#undef beast
#undef this_coro
#undef pbl
#undef prv
