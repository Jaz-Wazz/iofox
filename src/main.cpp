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
	io::stream stream {std::move(socket)};

	for(std::string cmd; std::getline(std::cin, cmd);)
	{
		if(cmd == "print")
		{
			stream.print_buffers();
		}
		if(cmd == "read")
		{
			// [Non buffered read]
			std::string buffer = std::string(32, '\0');
			std::size_t readed = co_await stream.async_read_some(asio::buffer(buffer), io::use_coro);
			fmt::print("readed: {} octets.\n", readed);
			io::log::print_hex_dump(buffer.data(), buffer.size());
		}
		if(cmd == "read_word")
		{
			std::string str;
			co_await (stream >> str);
			fmt::print("readed: '{}'.\n", str);
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
