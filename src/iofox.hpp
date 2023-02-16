#pragma once
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/beast/core/basic_stream.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/file.hpp>
#include <boost/beast/core/file_base.hpp>
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
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/system/detail/error_code.hpp>
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
#include <set>
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
		pbl using mode = beast::file_mode;

		pbl using beast::file::open;
		pbl using beast::file::write;
		pbl using beast::file::read;
		pbl using beast::file::pos;
		pbl using beast::file::size;
		pbl using beast::file::seek;
		pbl using beast::file::close;
		pbl using beast::file::native_handle;

		pbl auto open(const char * path, io::file::mode mode)
		{
			beast::error_code ec;
			beast::file::open(path, mode, ec);
			if(ec) throw std::system_error(ec);
		}

		pbl auto write(const void * buffer, std::size_t size) -> std::size_t
		{
			beast::error_code ec;
			auto ret = beast::file::write(buffer, size, ec);
			if(ec) throw std::system_error(ec); else return ret;
		}

		pbl auto read(void * buffer, std::size_t size) -> std::size_t
		{
			beast::error_code ec;
			auto ret = beast::file::read(buffer, size, ec);
			if(ec) throw std::system_error(ec); else return ret;
		}

		pbl auto pos() -> std::uint64_t
		{
			beast::error_code ec;
			auto ret = beast::file::pos(ec);
			if(ec) throw std::system_error(ec); else return ret;
		}

		pbl auto size() -> std::uint64_t
		{
			beast::error_code ec;
			auto ret = beast::file::size(ec);
			if(ec) throw std::system_error(ec); else return ret;
		}

		pbl void seek(std::uint64_t offset)
		{
			beast::error_code ec;
			beast::file::seek(offset, ec);
			if(ec) throw std::system_error(ec);
		}

		pbl void close()
		{
			beast::error_code ec;
			beast::file::close(ec);
			if(ec) throw std::system_error(ec);
		}

		pbl file() = default;
		pbl file(beast::file && file): beast::file(std::move(file)) {}
		pbl file(const char * path, io::file::mode mode) { open(path, mode); }
	};

	template <typename T>
	using channel = asio::experimental::channel<void(boost::system::error_code, T)>;

	template <typename T>
	class subscriber_channel;

	template <typename T>
	class broadcast_channel
	{
		friend io::subscriber_channel<T>;
		prv std::set<subscriber_channel<T> *> subscribers;
		prv asio::deadline_timer timer;
		pbl auto send(const T & i) -> io::coro<void>;
		prv typename std::set<subscriber_channel<T> *>::iterator it;
		pbl ~broadcast_channel();
		pbl broadcast_channel(auto & executor): timer(executor, boost::posix_time::pos_infin) {}
	};

	template <typename T>
	class subscriber_channel: public io::channel<T>
	{
		friend io::broadcast_channel<T>;
		prv io::broadcast_channel<T> * broadcast_channel;

		pbl subscriber_channel(io::broadcast_channel<T> & broadcast_channel, const asio::any_io_executor & executor)
		: broadcast_channel(&broadcast_channel), io::channel<T>(executor)
		{
			broadcast_channel.subscribers.insert(this);
			broadcast_channel.timer.cancel();
		}

		pbl ~subscriber_channel()
		{
			if(broadcast_channel != nullptr)
			{
				for(void * ptr : broadcast_channel->subscribers) fmt::print("-[{}] ", ptr);
				fmt::print("\n");
				broadcast_channel->it = broadcast_channel->subscribers.erase(broadcast_channel->subscribers.find(this));
				for(void * ptr : broadcast_channel->subscribers) fmt::print("[{}] ", ptr);
				fmt::print("\n");
			}
		}
	};

	template <typename T>
	inline auto broadcast_channel<T>::send(const T & i) -> io::coro<void>
	{
		if(subscribers.empty()) co_await timer.async_wait(io::use_coro_tuple);

		for(this->it = subscribers.begin(); this->it != subscribers.end(); this->it++)
		{
			for(void * ptr : subscribers) fmt::print("{}, ", ptr);
			fmt::print("\n");

			co_await (*this->it)->async_send({}, i, io::use_coro);
		}
	}

	template <typename T>
	inline broadcast_channel<T>::~broadcast_channel()
	{
		for(auto chan : subscribers) chan->broadcast_channel = nullptr;
	}
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
	// Concept check type is not std::nullopt and std::monostate.
	template <typename T> concept available = typeid(T) != typeid(std::nullopt) && typeid(T) != typeid(std::monostate);

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

		pbl template <typename X = T, typename std::enable_if<std::is_same_v<X, io::file>, int>::type = 0>
		request(std::string method, std::string target, header_list headers, io::file && body)
		: request(std::move(method), std::move(target), std::move(headers))
		{ this->body().file() = std::move(body); }

		pbl template <typename X = T, typename std::enable_if<std::is_same_v<X, beast::file>, int>::type = 0>
		request(std::string method, std::string target, header_list headers, beast::file && body)
		: request(std::move(method), std::move(target), std::move(headers))
		{ this->body().file() = std::move(body); }
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

		pbl template <typename X = T, typename std::enable_if<std::is_same_v<X, io::file>, int>::type = 0>
		response(unsigned int result, header_list headers, io::file && body): response(result, std::move(headers))
		{ this->body().file() = std::move(body); }

		pbl template <typename X = T, typename std::enable_if<std::is_same_v<X, beast::file>, int>::type = 0>
		response(unsigned int result, header_list headers, beast::file && body): response(result, std::move(headers))
		{ this->body().file() = std::move(body); }
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

		pbl request_header(beast::http::request_header<> && header): base(std::move(header)) {}
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

		pbl response_header(beast::http::response_header<> && header): base(std::move(header)) {}
	};

	// Basic high-level http/https client.
	class client
	{
		prv using tcp_stream = asio::ip::tcp::socket;
		prv using ssl_stream = asio::ssl::stream<asio::ip::tcp::socket>;

		prv class stage_read
		{
			pbl beast::http::response_parser<beast::http::buffer_body> parser;
			pbl beast::flat_buffer buffer;
			pbl stage_read(beast::http::response_header<> && header): parser(std::move(header)) { parser.body_limit(boost::none); }
		};

		prv class stage_write
		{
			pbl beast::http::request_serializer<beast::http::buffer_body> serializer;
			pbl beast::http::request<beast::http::buffer_body> request;
			pbl std::int64_t content_length = 0;
			pbl bool chunked = false;
			pbl stage_write(beast::http::request_header<> && header): request(std::move(header)), serializer(request) {}
		};

		prv class stage_connect
		{
			pbl io::url url;
			pbl stage_connect(io::url && url): url(std::move(url)) {}
		};

		prv std::variant<std::monostate, tcp_stream, ssl_stream> stream;
		prv std::variant<std::monostate, stage_read, stage_write, stage_connect> stage;
		pbl bool auto_host = true;

		pbl auto connect(io::url url) -> io::coro<void>
		{
			if(url.protocol == "http")
			{
				tcp_stream stream {co_await this_coro::executor};
				auto ips = co_await io::dns::resolve("http", url.host);
				co_await asio::async_connect(stream, ips, io::use_coro);
				this->stream = std::move(stream);
				stage.emplace<stage_connect>(std::move(url));
			}
			if(url.protocol == "https")
			{
				ssl_stream stream {co_await this_coro::executor, co_await io::ssl::context()};
				auto ips = co_await io::dns::resolve("https", url.host);
				co_await asio::async_connect(stream.next_layer(), ips, io::use_coro);
				io::ssl::set_tls_extension_hostname(stream, url.host);
				co_await stream.async_handshake(stream.client, io::use_coro);
				this->stream = std::move(stream);
				stage.emplace<stage_connect>(std::move(url));
			}
		}

		pbl auto write_header(io::http::request_header & header) -> io::coro<void>
		{
			if(auto_host && header["Host"].empty())
			{
				io::http::request_header temp_header {header.method_string().to_string(), header.target().to_string()};
				temp_header.set("Host", std::get<stage_connect>(stage).url.host);
				for(auto & el : header) temp_header.insert(el.name_string(), el.value());
				header = std::move(temp_header);
			}
			stage.emplace<stage_write>(std::move(header));

			co_await std::visit(meta::overloaded
			{
				[&](meta::available auto & stream, stage_write & stage) -> io::coro<void>
				{
					co_await beast::http::async_write_header(stream, stage.serializer, io::use_coro);
					stage.content_length = stage.request.has_content_length() ? std::stoll(stage.request["content-length"].to_string()) : 0;
					stage.chunked = stage.request.chunked();
					header = std::move(stage.request.base());
				},
				[](auto && ...) -> io::coro<void> { co_return; }
			}, stream, stage);
		}

		pbl auto write_header(beast::http::request_header<> & header) -> io::coro<void>
		{
			io::http::request_header inst {std::move(header)};
			co_await write_header(inst);
			header = std::move(inst);
		}

		pbl auto write_body_octets(const char * buffer, std::size_t size, bool last_buffer = false) -> io::coro<std::size_t>
		{
			co_return co_await std::visit(meta::overloaded
			{
				[&](meta::available auto & stream, stage_write & stage) -> io::coro<std::size_t>
				{
					if(stage.serializer.is_done()) co_return 0;
					if(stage.content_length > 0)
					{
						stage.request.body().data = const_cast<char *>(buffer);
						stage.request.body().size = (stage.content_length < size) ? stage.content_length : size;
						auto [err, writed] = co_await beast::http::async_write(stream, stage.serializer, io::use_coro_tuple);
						if(err.failed() && err != beast::http::error::need_buffer) throw std::system_error(err);
						stage.content_length -= writed;
						co_return writed;
					}
					if(stage.chunked)
					{
						stage.request.body().data = const_cast<char *>(buffer);
						stage.request.body().size = size;
						stage.request.body().more = !last_buffer;
						auto [err, writed] = co_await beast::http::async_write(stream, stage.serializer, io::use_coro_tuple);
						if(err.failed() && err != beast::http::error::need_buffer) throw std::system_error(err);
						co_return writed;
					}
					co_return 0;
				},
				[](auto && ...) -> io::coro<std::size_t> { co_return 0; },
			}, stream, stage);
		}

		pbl auto write_body_chunk_tail() -> io::coro<void>
		{
			co_await std::visit(meta::overloaded
			{
				[&](auto & stream) -> io::coro<void> { co_await asio::async_write(stream, beast::http::make_chunk_last(), io::use_coro); },
				[](std::monostate) -> io::coro<void> { co_return; }
			}, stream);
		}

		pbl auto write_body(std::string & string) -> io::coro<void>
		{
			co_await write_body_octets(string.data(), string.size(), true);
		}

		pbl auto write_body(meta::vector_one_byte auto & vector) -> io::coro<void>
		{
			co_await write_body_octets(vector.data(), vector.size(), true);
		}

		pbl auto write_body(io::file & file) -> io::coro<void>
		{
			for(char buf[4096]; auto readed = file.read(buf, 4096);)
			{
				auto writed = co_await write_body_octets(buf, readed, file.pos() == file.size());
				if(writed == 0) break;
			}
		}

		pbl auto write_body(beast::file & body) -> io::coro<void>
		{
			io::file file {std::move(body)};
			co_await write_body(file);
			body = std::move(file);
		}

		pbl auto write_body(beast::http::file_body::value_type & body) -> io::coro<void>
		{
			io::file file {std::move(body.file())};
			co_await write_body(file);
			body.file() = std::move(file);
		}

		pbl auto write_body(beast::http::empty_body::value_type & empty_body) -> io::coro<void> { co_return; }

		pbl template <typename T> auto write(io::http::request<T> & request) -> io::coro<void>
		{
			co_await write_header(request.base());
			co_await write_body(request.body());
		}

		pbl template <typename T> auto write(beast::http::request<T> & request) -> io::coro<void>
		{
			co_await write_header(request.base());
			co_await write_body(request.body());
		}

		pbl auto read_header(io::http::response_header & header) -> io::coro<void>
		{
			stage.emplace<stage_read>(std::move(header));

			co_await std::visit(meta::overloaded
			{
				[&](meta::available auto & stream, stage_read & stage) -> io::coro<void>
				{
					co_await beast::http::async_read_header(stream, stage.buffer, stage.parser, io::use_coro);
					header = std::move(stage.parser.get().base());
				},
				[](auto && ...) -> io::coro<void> { co_return; }
			}, stream, stage);
		}

		pbl auto read_header(beast::http::response_header<> & header) -> io::coro<void>
		{
			io::http::response_header inst {std::move(header)};
			co_await read_header(inst);
			header = std::move(inst);
		}

		pbl auto read_body_octets(char * buffer, std::size_t size) -> io::coro<std::size_t>
		{
			co_return co_await std::visit(meta::overloaded
			{
				[&](meta::available auto & stream, stage_read & stage) -> io::coro<std::size_t>
				{
					if(!stage.parser.is_done())
					{
						stage.parser.get().body().data = buffer;
						stage.parser.get().body().size = size;
						auto [err, bytes_readed] = co_await beast::http::async_read(stream, stage.buffer, stage.parser, io::use_coro_tuple);
						if(err.failed() && err != beast::http::error::need_buffer) throw std::system_error(err);
						co_return size - stage.parser.get().body().size;
					} else co_return 0;
				},
				[](auto && ...) -> io::coro<std::size_t> { co_return 0; },
			}, stream, stage);
		}

		pbl auto read_body(std::string & body) -> io::coro<void>
		{
			co_await std::visit(meta::overloaded
			{
				[&](stage_read & stage) -> io::coro<void>
				{
					if(auto content_length = stage.parser.content_length())
					{
						body.resize_and_overwrite(*content_length, [&](auto...) { return *content_length; });
						auto octets_readed = co_await read_body_octets(body.data(), *content_length);
						body.resize(octets_readed);
					}
					if(stage.parser.chunked())
					{
						for(int i = 0; true; i += 1024)
						{
							body.resize_and_overwrite(i + 1024, [&](auto...) { return i + 1024; });
							auto octets_readed = co_await read_body_octets(body.data() + i, 1024);
							if(octets_readed < 1024) { body.resize(i + octets_readed); break; }
						}
					}
					co_return;
				},
				[](auto && ...) -> io::coro<void> { co_return; }
			}, stage);
		}

		pbl auto read_body(meta::vector_one_byte auto & vector) -> io::coro<void>
		{
			co_await std::visit(meta::overloaded
			{
				[&](stage_read & stage) -> io::coro<void>
				{
					if(auto content_length = stage.parser.content_length())
					{
						vector.resize(*content_length);
						auto readed = co_await read_body_octets(vector.data(), *content_length);
						vector.resize(readed);
					}
					if(stage.parser.chunked())
					{
						for(int i = 0; true; i += 1024)
						{
							vector.resize(i + 1024);
							auto readed = co_await read_body_octets(vector.data() + i, 1024);
							if(readed < 1024) { vector.resize(i + readed); break; }
						}
					}
					co_return;
				},
				[](auto && ...) -> io::coro<void> { co_return; }
			}, stage);
		}

		pbl auto read_body(io::file & file) -> io::coro<void>
		{
			for(char buf[4096]; auto readed = co_await read_body_octets(buf, 4096);) co_await file.write(buf, readed);
		}

		pbl auto read_body(beast::file & file) -> io::coro<void>
		{
			io::file inst {std::move(file)};
			co_await read_body(inst);
			file = std::move(inst);
		}

		pbl auto read_body(beast::http::file_body::value_type & file_body) -> io::coro<void>
		{
			io::file inst {std::move(file_body.file())};
			co_await read_body(inst);
			file_body.file() = std::move(inst);
		}

		pbl auto read_body(beast::http::empty_body::value_type & empty_body) -> io::coro<void> { co_return; }

		pbl template <typename T> auto read(io::http::response<T> & response) -> io::coro<void>
		{
			co_await read_header(response.base());
			co_await read_body(response.body());
		}

		pbl template <typename T> auto read(beast::http::response<T> & response) -> io::coro<void>
		{
			co_await read_header(response.base());
			co_await read_body(response.body());
		}

		pbl void disconnect()
		{
			std::visit(meta::overloaded
			{
				[](tcp_stream & stream) { stream.shutdown(stream.shutdown_both); stream.close(); },
				[](ssl_stream & stream) { stream.next_layer().shutdown(tcp_stream::shutdown_both); stream.next_layer().close(); },
				[](std::monostate) {},
			}, stream);
		}
	};

	template <typename T = std::string>
	inline auto send(std::string method, io::url url, std::initializer_list<std::pair<std::string, std::string>> headers = {})
	-> io::coro<io::http::response<T>>
	{
		io::http::client client;
		co_await client.connect(url);
		io::http::request request {method, url.serialize_location(), headers};
		co_await client.write(request);
		io::http::response<T> response;
		co_await client.read(response);
		co_return response;
	}

	template <typename T = std::string>
	inline auto send(std::string method, io::url url, std::initializer_list<std::pair<std::string, std::string>> headers, const std::string & body)
	-> io::coro<io::http::response<T>>
	{
		io::http::client client;
		co_await client.connect(url);
		io::http::request<std::string> request {method, url.serialize_location(), headers, body};
		request.prepare_payload();
		co_await client.write(request);
		io::http::response<T> response;
		co_await client.read(response);
		co_return response;
	}
};

#undef asio
#undef beast
#undef this_coro
#undef pbl
#undef prv
