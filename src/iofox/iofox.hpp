#pragma once
#include "iofox/iofox.hpp"
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <ctype.h>
#include <exception>
#include <fmt/core.h>
#include <fmt/format.h>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <ranges>
#include <variant>
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

	inline void print_hex_dump(void * data, std::size_t size)
	{
		fmt::print("┌─────────────────────────────────────────────────────────────────────────────┐\n");
		for(auto chunk : io::log::hex_dump(data, size))
		{
			fmt::print("│ {} │ {} │ {} │\n", chunk.offset(), chunk.bytes(), chunk.chars());
		}
		fmt::print("└─────────────────────────────────────────────────────────────────────────────┘\n");
	}

	inline auto enumerate(auto range)
	{
		return std::views::zip(std::views::iota(std::size_t()), range);
	}

	inline auto print_read_cycle
	(
		asio::mutable_buffer buffer_0,
		asio::mutable_buffer buffer_1,
		asio::mutable_buffer buffer_2,
		std::string_view method,
		std::string_view path,
		std::span<phr_header> headers,
		int minor_version,
		int parser_code
	)
	{
		fmt::print("┌─────────────────────────────────────────────────────────────────────────────────────────┐\n");
		fmt::print("│ Read cycle                                                                      [iofox] │\n");
		fmt::print("├───────────┬─────────────────────────────────────────────────────────────────────────────┤\n");

		if(auto dump = io::log::hex_dump(buffer_0.data(), buffer_0.size()); !dump.empty())
		{
			for(auto [i, chunk] : io::log::enumerate(dump))
			{
				fmt::print("│ {:9} │ {} │ {} │ {} │\n", (i == 0) ? "Buffer 0:" : "", chunk.offset(), chunk.bytes(), chunk.chars());
			}
		}
		else fmt::print("│ {:9} │ {:75} │\n", "Buffer 0:", "Buffer empty.");

		fmt::print("├───────────┼─────────────────────────────────────────────────────────────────────────────┤\n");

		if(auto dump = io::log::hex_dump(buffer_1.data(), buffer_1.size()); !dump.empty())
		{
			for(auto [i, chunk] : io::log::enumerate(dump))
			{
				fmt::print("│ {:9} │ {} │ {} │ {} │\n", (i == 0) ? "Buffer 1:" : "", chunk.offset(), chunk.bytes(), chunk.chars());
			}
		}
		else fmt::print("│ {:9} │ {:75} │\n", "Buffer 1:", "Buffer empty.");

		fmt::print("├───────────┼─────────────────────────────────────────────────────────────────────────────┤\n");

		if(auto dump = io::log::hex_dump(buffer_2.data(), buffer_2.size()); !dump.empty())
		{
			for(auto [i, chunk] : io::log::enumerate(dump))
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
		} else fmt::print("│ {:9} │ {:75} │\n", "Buffer 2:", "Buffer empty.");

		fmt::print("├───────────┼─────────────────────────────────────────────────────────────────────────────┤\n");
		fmt::print("│ {:9} │ {:14} │ {:<58} │\n", "Parser:", "Method:", method);
		fmt::print("│ {:9} │ {:14} │ {:<58} │\n", "", "Path:", path);
		fmt::print("│ {:9} │ {:14} │ {:<58} │\n", "", "Minor version:", minor_version);
		fmt::print("│ {:9} │ {:14} │ {:<58} │\n", "", "Parser code:", parser_code);
		fmt::print("│           │ ─────────────────────────────────────────────────────────────────────────── │\n");

		if(!headers.empty())
		{
			for(auto && [i, header] : io::log::enumerate(headers))
			{
				auto line = fmt::format("{}: {}", std::string(header.name, header.name_len), std::string(header.value, header.value_len));
				fmt::print("│ {:9} │ {:14} │ {:<58} │\n", "", (i == 0) ? "Headers: " : "", line);
			}
		}
		else fmt::print("│ {:9} │ {:14} │ {:<58} │\n", "", "Headers: ", "Empty.");

		fmt::print("└───────────┴─────────────────────────────────────────────────────────────────────────────┘\n");
	}
};

namespace io
{
	class stream
	{
		prv asio::ip::tcp::socket socket;
		prv char buffer[128] {};
		prv std::size_t pos = 0;
		prv std::size_t size = 0;

		pbl stream(asio::ip::tcp::socket && socket)
		: socket(std::move(socket)) {}

		pbl auto next_layer() -> asio::ip::tcp::socket &
		{
			return socket;
		}

		pbl auto async_read_some(auto buffer, auto token)
		{
			return socket.async_read_some(buffer, token);
		}

		pbl void print_buffers()
		{
			io::log::print_hex_dump(buffer, pos);
			io::log::print_hex_dump(buffer + pos, size);
			io::log::print_hex_dump(buffer + pos + size, sizeof(buffer) - pos - size);
		}

		pbl auto operator >>(std::string & string) -> io::coro<void>
		{
			for(;;)
			{
				fmt::print("read internal buffer for object.\n");
				for(char c : std::span(buffer + pos, size))
				{
					pos++, size--;
					if(c != ' ') string += c; else co_return;
				}

				fmt::print("internal buffer is empty, but object is not complite. Read new chunk.\n");
				pos = 0, size = co_await socket.async_read_some(asio::buffer(buffer), io::use_coro);
			}
		}
	};

	class isstream: private asio::ip::tcp::socket
	{
		prv char buffer[8192] {};
		prv std::size_t pos = 0;
		prv std::size_t size = 0;

		pbl isstream(asio::ip::tcp::socket && socket)
		: asio::ip::tcp::socket(std::move(socket)) {}

		pbl auto next_layer() -> asio::ip::tcp::socket &
		{
			return *this;
		}

		pbl void print_buffers()
		{
			io::log::print_hex_dump(buffer, pos);
			io::log::print_hex_dump(buffer + pos, size);
			io::log::print_hex_dump(buffer + pos + size, sizeof(buffer) - pos - size);
		}

		prv std::size_t debug_size = 0;

		pbl auto async_fill() -> io::coro<void>
		{
			pos = 0, size = co_await async_read_some(asio::buffer(buffer), io::use_coro);
		}

		pbl auto get() -> std::optional<char>
		{
			if(size != 0)
			{
				pos++, size--;
				return buffer[pos - 1];
			}
			else return {};
		}
	};
}

#undef asio
#undef this_coro
#undef pbl
#undef prv
