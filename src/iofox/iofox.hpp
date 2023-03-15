#pragma once
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <exception>
#include <fmt/core.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <winnls.h>
#include <winnt.h>
#include <iofox/third_party/picohttpparser.h>

#define asio		boost::asio
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
	class start_line
	{
		pbl std::string method, path, version;

		pbl start_line(std::string method, std::string path, std::string version = "HTTP/1.1")
		: method(std::move(method)), path(std::move(path)), version(std::move(version)) {}

		pbl start_line(std::string_view string)
		{
			const char *	method_data		= nullptr;
			std::size_t		method_size		= 0;
			const char *	path_data		= nullptr;
			std::size_t		path_size		= 0;
			int				minor_version	= -1;
			std::size_t		headers_size	= 0;

			int ret = phr_parse_request
			(
				string.data(),
				string.size(),
				&method_data,
				&method_size,
				&path_data,
				&path_size,
				&minor_version,
				nullptr,
				&headers_size,
				0
			);

			if(ret == -2 && method_data != nullptr && path_data != nullptr && minor_version != -1)
			{
				method = {method_data, method_size};
				path = {path_data, path_size};
				version = (minor_version == 1) ? "HTTP/1.1" : "HTTP/1.0";
			}
			else throw std::runtime_error("io::http::start_line parse error");
		}

		pbl auto serialize() -> std::string
		{
			return fmt::format("{} {} {}", method, path, version);
		}
	};

	inline auto read_start_line() -> io::coro<void>
	{
		co_return;
	}
};

#undef asio
#undef this_coro
#undef pbl
#undef prv