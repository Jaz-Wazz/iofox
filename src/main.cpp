#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/buffer.hpp>
#include <cstring>
#include <fmt/core.h>
#include <iostream>
#include <string>
#include <vector>
#include <iofox/iofox.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto coro() -> io::coro<void>
{
	std::string str;
	auto buffer = asio::dynamic_buffer(str);
	fmt::print("String: '{}'.\n", str);

	{
		asio::mutable_buffer buf = buffer.prepare(5);
		std::memcpy(buf.data(), "garox", 5);
		fmt::print("String: '{}'.\n", str);
	}
	{
		buffer.commit(5);
		fmt::print("String: '{}'.\n", str);
	}
	{
		asio::const_buffer buf = buffer.data();
		fmt::print("Buffer data: '{}'.\n", std::string(static_cast<const char *>(buf.data()), buf.size()));
		fmt::print("String: '{}'.\n", str);
	}
	{
		buffer.grow(3);
		// buffer.consume(3);
		// buffer.shrink(3);
		fmt::print("String: '{}'.\n", str);
		asio::const_buffer buf = buffer.data();
		fmt::print("Buffer data: '{}'.\n", std::string(static_cast<const char *>(buf.data()), buf.size()));
	}


	// fmt::print("Buffer data: '{}'.\n", std::string(buffer.data()));
	// buffer.data()

	co_return;
}

int main() try
{
	io::windows::set_asio_locale(io::windows::lang::english);
	asio::io_context ctx;
	asio::co_spawn(ctx, coro(), io::rethrowed);
	return ctx.run();
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
