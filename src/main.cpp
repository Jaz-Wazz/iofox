#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/write.hpp>
#include <fmt/core.h>
#include <net_tails.hpp>
#include <twitch.hpp>
#include <util.hpp>
#include <iostream>

namespace asio = boost::asio;			// NOLINT.
namespace beast = boost::beast;			// NOLINT.
namespace http = beast::http;			// NOLINT.
namespace json = boost::json;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto coro() -> nt::sys::coro<void>
{
	io::http::client client;
	co_await client.connect("https://adbtc.top");

	http::request<http::empty_body> request {http::verb::get, "/", 11};
	request.set("host", "adbtc.top");
	co_await client.write(request);

	http::response<http::string_body> response;
	co_await client.read(response);

	std::cout << response;

	co_await client.disconnect();
}

int main() try
{
	nt::sys::windows::set_asio_message_locale(nt::sys::windows::lang::english);
	asio::io_context ctx;
	asio::co_spawn(ctx, coro(), nt::sys::rethrowed);
	return ctx.run();
}
catch(std::exception & e) { fmt::print("Exception: '{}'.\n", e.what()); }
catch(...) { fmt::print("Exception: 'unknown'.\n"); }
