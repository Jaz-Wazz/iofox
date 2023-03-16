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

class my_stream : private asio::ip::tcp::socket
{
	public: my_stream(asio::ip::tcp::socket && socket): asio::ip::tcp::socket(std::move(socket)) {}
	public: auto get_executor() { return asio::ip::tcp::socket::get_executor(); }

	public: using asio::ip::tcp::socket::executor_type;

	public: auto async_read_some(const auto & buffers, auto && token)
	{
		return asio::ip::tcp::socket::async_read_some(buffers, std::move(token));
	}
};

auto session(asio::ip::tcp::socket socket) -> io::coro<void>
{
	my_stream my_stream {std::move(socket)};
	fmt::print("Connected.\n");
	for(std::string cmd; std::getline(std::cin, cmd);)
	{
		if(cmd == "read_some")
		{
			std::string string;
			string.resize(4);
			std::size_t readed = co_await my_stream.async_read_some(asio::buffer(string), io::use_coro);
			fmt::print("Readed: {} octets, message: '{}'.\n", readed, string);
		}
		if(cmd == "read_until")
		{
			std::string string;
			// asio::async_read_until()
			// co_await asio::async_read_until(socket, asio::dynamic_buffer(string), "0", io::use_coro);
			std::size_t readed = co_await asio::async_read_until(my_stream, asio::dynamic_buffer(string), "0", io::use_coro);
			fmt::print("Readed: {} octets, message: '{}'.\n", readed, string);
		}
		if(cmd == "read_until_custom")
		{
			std::string b;
			auto x = asio::dynamic_buffer(b);

			// [123450fwewef]

			std::string inner_buffer;
			std::string outer_buffer;

			char buffer[10] {};

			std::size_t readed = co_await my_stream.async_read_some(asio::buffer(buffer), io::use_coro);
			fmt::print("Readed: {} octets, buffer: '{}'.\n", readed, std::string(buffer, readed));

			for(std::size_t i = 0; i < sizeof(buffer); i++)
			{
				if(buffer[i] != '0') outer_buffer.push_back(const char Ch)
			}

			// for(char c : buffer)
			// {
			// 	if(c != '0') outer_buffer.push_back(c);
			// }

			// std::string str = "12345sas1234";
			// auto x = str.find_first_of("sas");
			// fmt::print("Value: '{}'.\n", x);
		}
	}
}

auto coro() -> io::coro<void>
{
	asio::ip::tcp::acceptor acceptor {co_await this_coro::executor, {asio::ip::tcp::v4(), 80}};
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
