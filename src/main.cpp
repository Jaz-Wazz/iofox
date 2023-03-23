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

auto session(asio::ip::tcp::socket socket) -> io::coro<void>
{
	fmt::print("connected.\n");
	std::array<char, 32> buffer {};

	for(std::string cmd; std::getline(std::cin, cmd);)
	{
		if(cmd == "read_and_parse")
		{
			std::size_t readed = co_await socket.async_read_some(asio::buffer(buffer), io::use_coro);
			fmt::print("readed {} octets.\n", readed);
			fmt::print("buffer: '{}'.\n", std::string(buffer.data(), buffer.size()));

			const char *	method_data		= nullptr;
			std::size_t		method_size		= 0;
			const char *	path_data		= nullptr;
			std::size_t		path_size		= 0;
			int				minor_version	= -1;
			std::size_t		headers_size	= 0;

			int ret = phr_parse_request
			(
				buffer.data(),
				readed,
				&method_data,
				&method_size,
				&path_data,
				&path_size,
				&minor_version,
				nullptr,
				&headers_size,
				0
			);

			fmt::print("method: '{}'.\n", std::string(method_data, method_size));
			fmt::print("path: '{}'.\n", std::string(path_data, path_size));
			fmt::print("minor version: '{}'.\n", minor_version);

			if(ret > 0)		fmt::print("ret: '{}'.\n", "successfully parsed");
			if(ret == -1)	fmt::print("ret: '{}'.\n", "parse error");
			if(ret == -2)	fmt::print("ret: '{}'.\n", "request is incomplete");
		}
	}
}

auto coro() -> io::coro<void>
{
	asio::ip::tcp::acceptor acceptor {co_await this_coro::executor, {asio::ip::tcp::v4(), 555}};
    for(;;) asio::co_spawn(co_await this_coro::executor, session(co_await acceptor.async_accept(asio::use_awaitable)), io::rethrowed);
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
