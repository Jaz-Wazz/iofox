#pragma once
#include "iofox/iofox.hpp"
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
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
	inline char make_printable(char c)
	{
		return isprint(c) ? c : '.';
	};

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

		pbl auto bytes() -> std::string
		{
			return fmt::format("{:{}}", fmt::format("{:02x}", fmt::join(chunk, " ")), chunk_size * 3 - 1);
		}

		pbl auto chars() -> std::string
		{
			return fmt::format("{:{}}", fmt::format("{:c}", fmt::join(std::views::transform(chunk, make_printable), "")), chunk_size);
		}
	};

	inline auto hex_dump(void * data, std::size_t size, std::size_t chunk_size = 16)
	{
		auto make_hex_dump_chunk = [=](auto chunk){ return io::log::hex_dump_chunk(data, chunk, chunk_size); };
		return std::span(static_cast<char *>(data), size) | std::views::chunk(chunk_size) | std::views::transform(make_hex_dump_chunk);
	}

	inline auto enumerate(auto range)
	{
		return std::views::zip(std::views::iota(std::size_t()), range);
	}

	inline auto print_read_cycle(asio::mutable_buffer buffer_0, asio::mutable_buffer buffer_1, asio::mutable_buffer buffer_2)
	{
		fmt::print("┌─────────────────────────────────────────────────────────────────────────────────────────┐\n");
		fmt::print("│ Read cycle                                                                      [iofox] │\n");
		fmt::print("├───────────┬─────────────────────────────────────────────────────────────────────────────┤\n");

		auto dump_0 = io::log::hex_dump(buffer_0.data(), buffer_0.size());

		if(dump_0.size() == 0)
		{
			fmt::print("│ {:9} │ {:75} │\n", "Buffer 0:", "Buffer empty.");
		}

		for(auto [i, chunk] : io::log::enumerate(dump_0))
		{
			fmt::print("│ {:9} │ {} │ {} │ {} │\n", (i == 0) ? "Buffer 0:" : "", chunk.offset(), chunk.bytes(), chunk.chars());
		}

		fmt::print("├───────────┼─────────────────────────────────────────────────────────────────────────────┤\n");

		for(auto [i, chunk] : io::log::enumerate(io::log::hex_dump(buffer_1.data(), buffer_1.size())))
		{
			fmt::print("│ {:9} │ {} │ {} │ {} │\n", (i == 0) ? "Buffer 1:" : "", chunk.offset(), chunk.bytes(), chunk.chars());
		}

		fmt::print("├───────────┼─────────────────────────────────────────────────────────────────────────────┤\n");

		for(auto dump = io::log::hex_dump(buffer_2.data(), buffer_2.size()); auto [i, chunk] : io::log::enumerate(dump))
		{
			if(i < 5)
			{
				fmt::print("│ {:9} │ {} │ {} │ {} │\n", (i == 0) ? "Buffer 2:" : "", chunk.offset(), chunk.bytes(), chunk.chars());
			}
			else
			{
				fmt::print("│ {:9} │ {:6} │ {:47} │ {:16} │\n", "", "", fmt::format("And {} same lines...", dump.size() - 5), "");
				break;
			}
		}

		fmt::print("└─────────────────────────────────────────────────────────────────────────────────────────┘\n");
	}
};

#undef asio
#undef this_coro
#undef pbl
#undef prv
