#include <boost/asio/buffer.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <chrono>
#include <fmt/core.h>
#include <exception>
#include <fmt/format.h>
#include <iofox/iofox.hpp>
#include <iostream>
#include <ranges>
#include <string>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto generate_data_chunk() -> std::string
{
	std::string string;
	// for(int i : std::views::iota(0, 100000)) string += std::to_string(i) + ' ';
	for(int i : std::views::iota(0, 100'000'000))
	{
		string += fmt::format_int(i).c_str();
		string += ' ';
	}
	return string;
}

auto session(asio::ip::tcp::socket socket, std::string & data_chunk) -> io::coro<void>
{
	fmt::print("[session] - client connected.\n");
	std::size_t writed = co_await socket.async_write_some(asio::buffer(data_chunk), io::use_coro);
	fmt::print("[session] - sended {} octets.\n", writed);
	fmt::print("[session] - start endless waiting.\n\n", writed);
	co_await asio::steady_timer(socket.get_executor(), std::chrono::seconds::max()).async_wait(io::use_coro);
}

auto coro() -> io::coro<void>
{
	fmt::print("[coro] - generate data chunk...\n");
	std::string data_chunk = generate_data_chunk();
	fmt::print("[coro] - data chunk generated, size: '{}' bytes.\n", data_chunk.size());

	asio::ip::tcp::acceptor acceptor {co_await this_coro::executor, {asio::ip::tcp::v4(), 555}};
	for(;;)
	{
		auto socket = co_await acceptor.async_accept(asio::use_awaitable);
    	asio::co_spawn(co_await this_coro::executor, session(std::move(socket), data_chunk), io::rethrowed);
	}
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
