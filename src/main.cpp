#include <algorithm>
#include <array>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffered_read_stream.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/this_coro.hpp>
#include <cstring>
#include <ctype.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <iofox/iofox.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffered_stream.hpp>
#include <ranges>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

int main() try
{
	std::string str = "r2q3rqh23r8q2ht0q28rht4204\r\nithq230tq92th0q28tq2\r\nhy308rq2hr3h2r2q892\r\n34r98q2yr928q3ryh023r8r2rer";

	for(auto chunk : io::log::hex_dump(str.data(), str.size()))
	{
		fmt::print("| {} | {} | {} |\n", chunk.offset(), chunk.bytes(), chunk.chars());
	}

	// std::ranges::subrange<std::span<char>::iterator> s;

	// std::span span {str};
	// fmt::print("Size: '{}'.\n", x.size());

	// for(auto chunk : io::log::hex_dump(str.data(), str.size()))
	// {
	// 	fmt::print("| {} | {} |\n", chunk.bytes(), chunk.chars());
	// }

	// io::log::format_hex_dump("| {} | {} | {} |\n", str.data(), str.size(), 16);

	// io::log::custom_format_foo("inner: {} {}, my: {} {}.\n", 2, 3);

	// for(auto chunk : io::log::hex_dump()) fmt::print("", chunk.offset, chunk.bytes, chunk.chars);

	// fmt::print("{} {offset:} {}.\n", "svas", "sas", "sis", fmt::arg("offset", 4));

	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
