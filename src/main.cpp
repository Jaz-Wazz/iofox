#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>
#include <fmt/core.h>
#include <net_tails.hpp>
#include <twitch.hpp>
#include <util.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace beast = boost::beast;			// NOLINT.
namespace http = beast::http;			// NOLINT.
namespace json = boost::json;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

void test()
{
	{
		nt::url url = "https://exmaple.com/page/sas/garox.php?key=value&foo=bar#firstheader";
		fmt::print("Protocol: {}\n", url.protocol);
		fmt::print("Host:     {}\n", url.host);
		fmt::print("Path:     {}\n", url.path);
		fmt::print("Query:    {}\n", url.query);
		fmt::print("Fragment: {}\n", url.fragment);
		fmt::print("Location: {}\n", url.serialize_location());
	}
	{
		nt::url url {"ftp", "garox.com", "page.html", "sas=dudos", "tag"};
		fmt::print("Url 2: {}\n", url.serialize());
	}
	{
		nt::url url;
		url.protocol = "ftp";
		url.host = "garox.com";
		url.path = "sas/garox/dudos";
		url.query = "que=ery&qq=qe";
		url.fragment = "frag";
		fmt::print("Url 3: {}\n", url.serialize());
	}
}

auto coro() -> nt::sys::coro<void>
{
	co_return;
}

int main() try
{
	test();
	// nt::sys::windows::set_asio_message_locale(nt::sys::windows::lang::english);
	// asio::io_context ctx;
	// asio::co_spawn(ctx, coro(), nt::sys::rethrowed);
	// return ctx.run();
}
catch(std::exception & e) { fmt::print("Exception: '{}'.\n", e.what()); }
catch(...) { fmt::print("Exception: 'unknown'.\n"); }
