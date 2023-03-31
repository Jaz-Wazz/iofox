#include <cstddef>
#include <cstring>
#include <iofox/third_party/picohttpparser.h>
#include <boost/asio/buffer.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/registered_buffer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <fmt/core.h>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <iofox/iofox.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

#define prv private:
#define pbl public:

class static_dynamic_buffer
{
	prv char buffer[128] = {};
	prv std::size_t size_zone_consumed = 0;
	prv std::size_t size_zone_data = 0;

	pbl auto size()		-> std::size_t	{ return size_zone_data;						} // Data chunk size.
	pbl auto max_size()	-> std::size_t	{ return sizeof(buffer) - size_zone_consumed;	} // Max possibly size.
	pbl auto capacity()	-> std::size_t	{ return sizeof(buffer) - size_zone_consumed;	} // Max size without allocation.

	pbl auto grow(std::size_t n)
	{
		if(size() + n < max_size()) size_zone_data += n; else throw std::length_error("buffer over-grow");
	}

	pbl auto data(std::size_t pos, std::size_t n) -> asio::mutable_buffer
	{
		return {buffer + size_zone_consumed, (n < size()) ? n : size()};
	}

	pbl auto consume(std::size_t n)
	{
		size_zone_consumed += n;
		size_zone_data -= n;
	}
};

#undef prv
#undef pbl

int main() try
{
	// stream << request | []{ /* request complite */ };
	// co_await (stream << request | io::use_coro);
	// co_await stream.async_write(request, io::use_coro + timeout(6));
	// if(5 | 6) {}

	static_dynamic_buffer dynamic_buffer;

	for(std::string cmd; std::getline(std::cin, cmd);)
	{
		if(cmd == "data")
		{
			asio::mutable_buffer buffer = dynamic_buffer.data(0, dynamic_buffer.size());
			io::log::print_hex_dump(buffer.data(), buffer.size());
		}
		if(cmd == "consume")
		{
			dynamic_buffer.consume(2);
		}
		if(cmd == "write")
		{
			dynamic_buffer.grow(2);
			asio::mutable_buffer buffer = dynamic_buffer.data(dynamic_buffer.size() - 2, 2);
			std::memcpy(buffer.data(), "xy", 2);
		}
		if(cmd == "info")
		{
			fmt::print("size: {}\n",		dynamic_buffer.size());
			fmt::print("max size: {}\n",	dynamic_buffer.max_size());
			fmt::print("capacity: {}\n",	dynamic_buffer.capacity());
		}
	}

	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
