#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/file.hpp>
#include <boost/beast/core/file_base.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/serializer.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/vector_body.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/system/detail/error_code.hpp>
#include <fmt/core.h>
#include <iofox.hpp>
#include <twitch.hpp>
#include <util.hpp>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

namespace asio = boost::asio;			// NOLINT.
namespace beast = boost::beast;			// NOLINT.
namespace http = beast::http;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto coro_test() -> io::coro<void>
{
	// connect.
	asio::ssl::stream<asio::ip::tcp::socket> stream {co_await this_coro::executor, co_await io::ssl::context()};
	co_await asio::async_connect(stream.next_layer(), co_await io::dns::resolve("https", "webhook.site"), io::use_coro);
	io::ssl::set_tls_extension_hostname(stream, "webhook.site");
	co_await stream.async_handshake(stream.client, io::use_coro);

	// write.
	beast::http::request<beast::http::string_body> request {beast::http::verb::post, "/0b3c3220-de18-4166-bf40-f9aa7e35a6db", 11};
	request.set("host", "webhook.site");
	request.set("Transfer-Encoding", "chunked");
	request.body() = "very_long_message_with_very_big_data_for_test";
	beast::http::request_serializer<beast::http::string_body> serializer {request};
	co_await beast::http::async_write(stream, serializer, io::use_coro);

	// read.
	beast::flat_buffer buf;
	io::http::response<std::string> response;
	co_await beast::http::async_read(stream, buf, response, io::use_coro);

	// log.
	std::cout << response << '\n';
}

auto coro_test_2() -> io::coro<void>
{
	// connect.
	asio::ssl::stream<asio::ip::tcp::socket> stream {co_await this_coro::executor, co_await io::ssl::context()};
	co_await asio::async_connect(stream.next_layer(), co_await io::dns::resolve("https", "webhook.site"), io::use_coro);
	io::ssl::set_tls_extension_hostname(stream, "webhook.site");
	co_await stream.async_handshake(stream.client, io::use_coro);

	// write.
	beast::http::request<beast::http::buffer_body> request {beast::http::verb::post, "/0b3c3220-de18-4166-bf40-f9aa7e35a6db", 11};
	request.set("host", "webhook.site");
	request.set("Transfer-Encoding", "chunked");
	// request.set("Content-Length", "45");

	std::string data = "very_long_message_with_very_big_data_for_test";
	request.body().data = data.data();
	request.body().size = data.length();
	request.body().more = false;

	beast::http::request_serializer<beast::http::buffer_body> serializer {request};
	auto [err, bytes_writed] = co_await beast::http::async_write(stream, serializer, asio::as_tuple(io::use_coro));
	if(err.failed() && err != beast::http::error::need_buffer) throw std::system_error(err);

	// read.
	beast::flat_buffer buf;
	io::http::response<std::string> response;
	co_await beast::http::async_read(stream, buf, response, io::use_coro);

	// log.
	std::cout << response << '\n';
}

auto coro() -> io::coro<void>
{
	io::http::client client;
	co_await client.connect("https://webhook.site");

	std::string body = "very_long_message_with_very_big_data_for_test";

	io::http::request_header request_header {"POST", "/0b3c3220-de18-4166-bf40-f9aa7e35a6db",
	{
		{"host", "httpbin.org"},
		{"content-length", std::to_string(body.length())}
		// {"Transfer-Encoding", "chunked"}
	}};
	co_await client.write_header(request_header);

	for(int i = 0; i < body.length(); i += 4)
	{
		auto slice = body.substr(i, 4);
		co_await client.write_body_piece(slice.data(), slice.size());
		fmt::print("write: '{}'.\n", slice);
	}
	// co_await client.write_body_piece_tail();

	// x.write_octets();
	// x.write_end();

	// client.write();

	// client.write_header();
	// client.write_body();

	// client.write_body_piece();
	// client.write_body_piece_end();

	//	for(;;)
	//	{
	//		char buffer[8] = "12345678";
	//		co_await client.write_body_piece(buffer, 8);
	//	}
	//	co_await client.write_body_chunk_end();

	//	for(int i = 0; i <= 10; i++)
	//	{
	//		char buffer[8] = "12345678";
	//		co_await client.write_body_piece(buffer, 8, (i == 10) ? true : false);
	//	}

	//	char buffer[8];
	//	while(auto bytes_readed = co_await client.read_body_piece(buffer, 8))
	//	{
	//		std::cout << "Readed: " << bytes_readed << " bytes."
	//	}

	io::http::response<std::string> response;
	co_await client.read(response);

	std::cout << response << '\n';
}

int main() try
{
	io::windows::set_asio_locale(io::windows::lang::english);
	asio::io_context ctx;
	asio::co_spawn(ctx, coro(), io::rethrowed);
	return ctx.run();
}
catch(std::exception & e) { fmt::print("Exception: '{}'.\n", e.what()); }
catch(...) { fmt::print("Exception: 'unknown'.\n"); }
