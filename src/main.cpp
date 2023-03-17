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
	std::string string;
	auto dynamic_buffer = asio::dynamic_buffer(string);

	for(std::string cmd; std::getline(std::cin, cmd);)
	{
		if(cmd == "show")
		{
			asio::const_buffer buffer = dynamic_buffer.data();
			fmt::print("Buffer: '{}'.\n", std::string(static_cast<const char *>(buffer.data()), buffer.size()));
		}
		if(cmd == "add")
		{
			asio::mutable_buffer buffer = dynamic_buffer.prepare(5);
			std::memcpy(buffer.data(), "garox", 5);
		}
		if(cmd == "commit")
		{
			dynamic_buffer.commit(1);
		}
		if(cmd == "grow")
		{
			dynamic_buffer.grow(1);
		}
	}

	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
