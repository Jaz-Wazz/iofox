#pragma once
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <ctype.h>
#include <exception>
#include <fmt/core.h>
#include <fmt/format.h>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <ranges>
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

namespace io::log
{
	template <typename... T> void custom_format_foo(fmt::format_string<char, int, T...> fmt, T && ... args)
	{
		fmt::print(fmt, 's', 77, std::forward<T>(args)...);
	}

	inline auto format_hex_dump(void * data, std::size_t size, std::size_t chunk_size = 16)
	{
		auto make_printable = [](char c){ return isprint(c) ? c : '.'; };
		auto xdata = static_cast<char *>(data);

		for(auto chunk : std::span(xdata, size) | std::views::chunk(chunk_size))
		{
			auto offset = fmt::format("{:06x}", chunk.data() - xdata);
			auto bytes = fmt::format("{:{}}", fmt::format("{:02x}", fmt::join(chunk, " ")), chunk_size * 3 - 1);
			auto chars = fmt::format("{:{}}", fmt::format("{:c}", fmt::join(std::views::transform(chunk, make_printable), "")), chunk_size);

			fmt::print("| {} | {} | {} |\n", offset, bytes, chars);
		}
	}

	class hex_dump_chunk
	{
		prv std::span<char> chunk;
		prv char * data;
		prv std::size_t chunk_size;

		pbl hex_dump_chunk(void * data, std::span<char> chunk, std::size_t chunk_size)
		: data(static_cast<char *>(data)), chunk(std::move(chunk)), chunk_size(chunk_size) {}

		pbl auto offset() -> std::string
		{
			return fmt::format("{:06x}", chunk.data() - data);
		}

		pbl auto bytes(std::string_view separator = " ") -> std::string
		{
			return fmt::format("{:{}}", fmt::format("{:02x}", fmt::join(chunk, separator)), chunk_size * 3 - 1);
		}

		pbl auto chars(std::string_view separator = "") -> std::string
		{
			auto make_printable = [](char c){ return isprint(c) ? c : '.'; };
			return fmt::format("{:{}}", fmt::format("{:c}", fmt::join(std::views::transform(chunk, make_printable), separator)), chunk_size);
		}
	};

	inline auto hex_dump(void * data, std::size_t size, std::size_t chunk_size = 16)
	{
		return std::span<char>(static_cast<char *>(data), size) | std::views::chunk(chunk_size)
		| std::views::transform([=](auto chunk){ return io::log::hex_dump_chunk(data, chunk, chunk_size); });
	}
};

#undef asio
#undef this_coro
#undef pbl
#undef prv
