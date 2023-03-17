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
#include <fmt/core.h>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <iofox/iofox.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffered_stream.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

int main() try
{
	const std::string str = "sas";
	asio::const_buffer const_buffer = asio::buffer(str);
	fmt::print("ptr: '{}', size: '{}'.\n", const_buffer.data(), const_buffer.size());

	const_buffer = asio::const_buffer(nullptr, 0);
	fmt::print("ptr: '{}', size: '{}'.\n", const_buffer.data(), const_buffer.size());

	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
