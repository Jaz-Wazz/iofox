#include <array>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffered_read_stream.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/beast/core/buffered_read_stream.hpp>
#include <cstring>
#include <fmt/core.h>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <iofox/iofox.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffered_stream.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.
namespace beast = boost::beast;			// NOLINT.

int main() try
{
	std::string string;
	auto dynamic_buffer = asio::dynamic_buffer(string);

	for(std::string cmd; std::getline(std::cin, cmd);)
	{
		if(cmd == "show")
		{
			asio::mutable_buffer buffer = dynamic_buffer.data(0, dynamic_buffer.size());
			fmt::print("Buffer data: '{}'.\n", std::string(static_cast<char *>(buffer.data()), buffer.size()));
		}
		if(cmd == "add")
		{
			std::string message_to_write = "message_to_write";
			dynamic_buffer.grow(message_to_write.size());
			asio::mutable_buffer buffer = dynamic_buffer.data(dynamic_buffer.size() - message_to_write.size(), message_to_write.size());
			std::memcpy(buffer.data(), message_to_write.data(), message_to_write.size());
			fmt::print("Buffer ptr: '{}', size: '{}'.\n", buffer.data(), buffer.size());
		}
		if(cmd == "consume")
		{
			dynamic_buffer.consume(3);
		}
		if(cmd == "shrink")
		{
			dynamic_buffer.shrink(3);
		}
	}

	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
