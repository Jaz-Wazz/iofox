#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>
#include <fmt/core.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <iofox/iofox.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

int main() try
{
	std::string str = "r2q3rqh23r8q2ht0q28rht4204\r\nithq230tq92th0q28tq2\r\nhy308rq2hr3h2r2q892\r\n34r98q2yr928q3ryh023r8r2rer";

	for(auto chunk : io::log::hex_dump(str.data(), str.size()))
	{
		fmt::print("| {} | {} | {} |\n", chunk.offset(), chunk.bytes(), chunk.chars());
	}

	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
