#include <boost/asio/buffer.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/write.hpp>
#include <boost/buffers/const_buffer.hpp>
#include <boost/http_proto/request_parser.hpp>
#include <boost/http_proto/request.hpp>
#include <boost/http_proto/response.hpp>
#include <boost/http_proto/serializer.hpp>
#include <boost/http_proto/status.hpp>
#include <boost/http_proto/string_body.hpp>
#include <boost/http_proto/version.hpp>
#include <exception>
#include <fmt/core.h>
#include <iostream>
#include <string>
#include <vector>
#include <iofox.hpp>

namespace http_proto = boost::http_proto;	// NOLINT.
namespace asio = boost::asio;				// NOLINT.
namespace this_coro = asio::this_coro; 		// NOLINT.

auto coro() -> io::coro<void>
{
	asio::ip::tcp::socket sock {co_await this_coro::executor};
	co_await asio::async_connect(sock, co_await io::dns::resolve("http", "exmaple.com"), io::use_coro);
	fmt::print("connected.\n");

	http_proto::request request;
	request.set_start_line("GET", "/", http_proto::version::http_1_1);
	request.set("Host", "exmaple.com");
	request.set("Connection", "close");
	fmt::print("request:\n{}\n", request.buffer());

	auto ret = co_await asio::async_write(sock, asio::buffer(request.buffer()), io::use_coro);
	fmt::print("writed: '{}'.\n", ret);

	// std::string buf;
	// buf.resize(100);
	// co_await asio::async_read(sock, asio::buffer(buf), io::use_coro);
	// fmt::print("readed:\n{}\n", buf);

	// std::string buf;
	// co_await asio::async_read(sock, asio::dynamic_buffer(buf), io::use_coro);
	// fmt::print("readed:\n{}\n", buf);

	std::string str;
	co_await asio::async_read_until(sock, asio::dynamic_buffer(str), "\r\n", io::use_coro);
	fmt::print("readed:\n{}\n", str);

	std::string str2;
	co_await asio::async_read_until(sock, asio::dynamic_buffer(str2), "\r\n", io::use_coro_tuple);
	fmt::print("readed:\n{}\n", str2);
}

int main() try
{
	asio::io_context ctx;
	asio::co_spawn(ctx, coro(), io::rethrowed);
	return ctx.run();
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
