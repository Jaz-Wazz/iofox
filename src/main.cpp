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

// ------------------
// GET /page.html HTTP/1.1
// Host: site.com
// body
// ---------------------

// auto request = read_request(socket);

// auto headers = read_headers(socket);
// auto body = read_body(socket);

// buffer buffer;
// auto headers = read_headers(socket, buffer);
// auto body = read_body(socket, buffer);

// auto headers = read_headers(buffered_socket, buffer);
// auto body = read_body(buffered_socket, buffer);

// parser << socket;
// parser << socket;
// parser >> headers;

// std::ifstream stream {"file.txt"};
// int i;
// stream >> i;
// stream >> i;
// stream >> i;
// stream >> i;
// stream >> i;
// stream >> i;
// stream >> i;
// stream >> i;
// stream >> i;

// [file] -> [stream_buffer] -> [user_object].


// buffered_socket >> object;
// buffered_socket.read(object, size);

// socket.read(4);


// io::http::request request {socket};
// io::http::body<std::string> body {socket};



// io::http_stream stream;
//
// io::http::request<std::string> request;
// stream >> request;

// io::http_stream stream;
//
// io::http::request request;
// stream >> request;
//
// std::string body;
// strram >> body;
//
// io::http::response response;
// stream << response;

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
	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
