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
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <iofox/iofox.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffered_stream.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto pico_err_to_string(int code) -> std::string
{
	if(code > 0)	return "successfully parsed";
	if(code == -1)	return "parse error";
	if(code == -2)	return "request is incomplete";
	throw std::runtime_error("unknown err pico code");
}

auto fix_strings(std::string string) -> std::string
{
	std::string out;
	for(char c : string) if(c == '\r') out += "\\r"; else if(c == '\n') out += "\\n"; else out += c;
	return out;
}

auto buf_to_str(auto buffer) -> std::string
{
	return {static_cast<const char *>(buffer.data()), buffer.size()};
}

void hexdump(void * ptr, std::size_t size)
{
	unsigned char * buf = static_cast<unsigned char *>(ptr);
	for (int i = 0; i < size; i += 16)
	{
		fmt::print("│ {:06x} │ ", i);
		for (int j = 0; j < 16; j++) if (i + j < size) fmt::print("{:02x} ", buf[i + j]); else fmt::print("   ");
		fmt::print("│ ");
		for (int j = 0; j < 16; j++) if (i + j < size) fmt::print("{:c}", isprint(buf[i + j]) ? buf[i + j] : '.'); else fmt::print(" ");
		fmt::print(" │\n");
	}
}

auto session(asio::ip::tcp::socket socket) -> io::coro<void>
{
	fmt::print("connected.\n");
	char buffer[8192] {};
	std::size_t buffer_size = 0;
	std::size_t prev_buffer_size = 0;

	for(std::string cmd; std::getline(std::cin, cmd);)
	{
		if(cmd == "read")
		{
			std::size_t readed = co_await socket.async_read_some(asio::buffer(buffer + buffer_size, sizeof(buffer) - buffer_size), io::use_coro);

			auto buffer_0 = asio::buffer(buffer, buffer_size);
			auto buffer_1 = asio::buffer(buffer + buffer_size, readed);
			auto buffer_2 = asio::buffer(buffer + buffer_size + readed, sizeof(buffer) - buffer_size - readed);
			buffer_size += readed;

			fmt::print("┌─────────────────────────────────────────────────────────────────────────────┐\n");
			fmt::print("│ Buffer dump                                                                 │\n");
			fmt::print("├─────────────────────────────────────────────────────────────────────────────┤\n");
			hexdump(buffer_0.data(), buffer_0.size());
			fmt::print("├─────────────────────────────────────────────────────────────────────────────┤\n");
			hexdump(buffer_1.data(), buffer_1.size());
			fmt::print("├─────────────────────────────────────────────────────────────────────────────┤\n");
			hexdump(buffer_2.data(), buffer_2.size());
			fmt::print("└─────────────────────────────────────────────────────────────────────────────┘\n");

			// fmt::print("[socket_reader] - [{}] + [{}] + [{}].\n", buf_to_str(buffer_0), buf_to_str(buffer_1), buf_to_str(buffer_2));

			// fmt::print("┌─────────────────────────────────────┐\n");
			// fmt::print("│ Read                                │\n");
			// fmt::print("├─────────────────────────────────────┤\n");
			// fmt::print("│ [{}] + [{}] + [{}].\n", buf_to_str(buffer_0), buf_to_str(buffer_1), buf_to_str(buffer_2));

			// fmt::print("[socket_reader] - {:<16} -> '{}'.\n", "full buffer",		fix_strings({buffer, buffer_size + readed}));
			// fmt::print("[socket_reader] - {:<16} -> '{}'.\n", "readed chunk",		fix_strings({buffer + buffer_size, readed}));
			// buffer_size += readed;
			// fmt::print("[socket_reader] - {:<16} -> '{}'.\n", "readed size",		buffer_size);
			// fmt::print("[socket_reader] - {:<16} -> '{}'.\n", "prev readed size",	prev_buffer_size);

			// const char *	method_data		= nullptr;
			// std::size_t		method_size		= 0;
			// const char *	path_data		= nullptr;
			// std::size_t		path_size		= 0;
			// int				minor_version	= -1;
			// std::size_t		headers_size	= 0;

			// int ret = phr_parse_request
			// (
			// 	buffer,
			// 	buffer_size,
			// 	&method_data,
			// 	&method_size,
			// 	&path_data,
			// 	&path_size,
			// 	&minor_version,
			// 	nullptr,
			// 	&headers_size,
			// 	prev_buffer_size
			// );

			// prev_buffer_size += readed;

			// fmt::print("[pico_parser] - {:<15} -> '{}'.\n", "method",			std::string(method_data, method_size));
			// fmt::print("[pico_parser] - {:<15} -> '{}'.\n", "path",				std::string(path_data, path_size));
			// fmt::print("[pico_parser] - {:<15} -> '{}'.\n", "minor version",	minor_version);
			// fmt::print("[pico_parser] - {:<15} -> '{}'.\n", "status",			pico_err_to_string(ret));
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
