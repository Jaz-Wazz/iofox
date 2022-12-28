#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>
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
	// String body.
	{
		io::http::client client;
		co_await client.connect("https://adbtc.top");

		io::http::request<> request {"GET", "/", {{"host", "adbtc.top"}}};
		co_await client.write(request);

		io::http::response<std::string> response;
		co_await client.read(response);
		std::cout << response << '\n';
	}

	// File body.
	{
		io::http::client client;
		co_await client.connect("https://adbtc.top");

		io::http::request<> request {"GET", "/", {{"host", "adbtc.top"}}};
		co_await client.write(request);

		http::response<http::file_body> response;
		boost::system::error_code e;
		response.body().open("sas.txt", beast::file_mode::write, e);
		fmt::print("{}\n", e.message());
		co_await client.read(response);
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
