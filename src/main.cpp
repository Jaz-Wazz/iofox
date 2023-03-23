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
#include <boost/container/static_vector.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

int main() try
{
	boost::container::static_vector<char, 32> static_vector;

	for(std::string cmd; std::getline(std::cin, cmd);)
	{
		if(cmd == "show")
		{
			fmt::print("vector data: '{}'.\n", std::string(static_vector.data(), static_vector.size()));
		}
		if(cmd == "add")
		{
			static_vector.push_back('s');
			static_vector.push_back('a');
			static_vector.push_back('s');
		}
		if(cmd == "remove")
		{
			static_vector.pop_back();
		}
	}

	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
