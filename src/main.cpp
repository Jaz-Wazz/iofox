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

auto session(asio::ip::tcp::socket socket) -> io::coro<void>
{
	fmt::print("connected.\n");
	char buffer[8192] {};
	asio::mutable_buffer buffer_0, buffer_1, buffer_2 = asio::buffer(buffer);

	for(std::string cmd; std::getline(std::cin, cmd);)
	{
		if(cmd == "read")
		{
			std::size_t readed = co_await socket.async_read_some(buffer_2, io::use_coro);

			buffer_0 = asio::buffer(buffer, buffer_0.size());
			buffer_1 = asio::buffer(buffer + buffer_0.size(), readed);
			buffer_2 = asio::buffer(buffer + buffer_0.size() + readed, sizeof(buffer) - buffer_0.size() - readed);

			const char *	method_data		= nullptr;
			std::size_t		method_size		= 0;
			const char *	path_data		= nullptr;
			std::size_t		path_size		= 0;
			int				minor_version	= -1;
			phr_header		headers[3]		= {};
			std::size_t		headers_size	= sizeof(headers);

			int ret = phr_parse_request
			(
				buffer,
				buffer_0.size() + buffer_1.size(),
				&method_data,
				&method_size,
				&path_data,
				&path_size,
				&minor_version,
				headers,
				&headers_size,
				buffer_0.size()
			);

			io::log::print_read_cycle
			(
				buffer_0,
				buffer_1,
				buffer_2,
				{method_data, method_size},
				{path_data, path_size},
				{headers, headers_size},
				minor_version,
				ret
			);

			buffer_0 = asio::buffer(buffer_0.data(), buffer_0.size() + readed);
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
