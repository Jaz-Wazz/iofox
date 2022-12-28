#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/beast/core/file.hpp>
#include <boost/beast/core/file_base.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/system/detail/error_code.hpp>
#include <fmt/core.h>
#include <iofox.hpp>
#include <twitch.hpp>
#include <util.hpp>
#include <iostream>

namespace asio = boost::asio;			// NOLINT.
namespace beast = boost::beast;			// NOLINT.
namespace http = beast::http;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto coro() -> io::coro<void>
{
	// std::string.
	{
		io::http::client client;
		co_await client.connect("https://adbtc.top");

		io::http::request<> request {"GET", "/", {{"host", "adbtc.top"}}};
		co_await client.write(request);

		io::http::response_header response_header;
		co_await client.read_header(response_header);
		std::cout << response_header << '\n';

		http::string_body::value_type str;
		co_await client.read_body(str);
		std::cout << str << '\n';
	}

	// file.
	{
		io::http::client client;
		co_await client.connect("https://adbtc.top");

		io::http::request<> request {"GET", "/", {{"host", "adbtc.top"}}};
		co_await client.write(request);

		io::http::response_header response_header;
		co_await client.read_header(response_header);
		std::cout << response_header << '\n';

		http::file_body::value_type body;
		boost::system::error_code e;
		body.open("sas.txt", beast::file_mode::write, e);
		co_await client.read_body(body);
	}

	//file 2.
	{
		io::http::client client;
		co_await client.connect("https://adbtc.top");

		io::http::request<> request {"GET", "/", {{"host", "adbtc.top"}}};
		co_await client.write(request);

		io::http::response_header response_header;
		co_await client.read_header(response_header);
		std::cout << response_header << '\n';

		beast::file body;
		boost::system::error_code e;
		body.open("sas.txt", beast::file_mode::write, e);
		co_await client.read_body(body);
	}
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
