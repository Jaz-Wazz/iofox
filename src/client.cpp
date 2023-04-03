#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <fmt/core.h>
#include <exception>
#include <iofox/iofox.hpp>
#include <ranges>
#include <string_view>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto read_one_buffer(asio::ip::tcp::socket socket) -> io::coro<void>
{
	char * data = new char[888888890];
	std::size_t size = 0;

	for(;size != 888888890;)
	{
		std::size_t readed = co_await socket.async_read_some(asio::buffer(data + size, 888888890 - size), io::use_coro);
		size += readed;
		fmt::print("[reader] - readed {:16} bytes, size {:16} bytes.\n", readed, size);
	}

	for(std::span chunk : std::span(data, size) | std::views::chunk(8192))
	{
		fmt::print("{}\n", std::string_view(chunk.data(), chunk.size()));
	}
};

auto coro() -> io::coro<void>
{
	asio::ip::tcp::socket socket {co_await this_coro::executor};
	co_await socket.async_connect({asio::ip::make_address("127.0.0.1"), 555}, io::use_coro);
	fmt::print("[client] - connected.\n");

	co_await read_one_buffer(std::move(socket));
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
