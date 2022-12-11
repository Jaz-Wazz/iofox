#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/json/array.hpp>
#include <boost/json/object.hpp>
#include <boost/json/serialize.hpp>
#include <boost/json/value.hpp>
#include <fstream>
#include <iostream>
#include <regex>
#include <fmt/core.h>
#include <net_tails.hpp>

namespace asio = boost::asio;			// NOLINT.
namespace beast = boost::beast;			// NOLINT.
namespace http = beast::http;			// NOLINT.
namespace json = boost::json;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto json_format(const auto && str) -> std::string
{
	// Copy string.
	std::string x = str;

	// Iterate each char.
	for(int i = 0, tabs = 0; i <= x.length(); i++)
	{
		if(x[i] == ',')
		{
			// Move content after ',' to new line and tabulate.
			x.insert(i + 1, "\n");
			x.insert(i + 2, tabs, '\t');

			// Move index.
			i += 1 + tabs;
		}
		if(x[i] == '{' || x[i] == '[')
		{
			// Increasure tabs.
			tabs++;

			// Move content after '{' to new line and tabulate.
			x.insert(i + 1, "\n");
			x.insert(i + 2, tabs, '\t');

			// Move index.
			i += 1 + tabs;
		}
		if(x[i] == '}' || x[i] == ']')
		{
			// Decreasure tabs.
			tabs--;

			// Move '}' to new line and tabulate.
			x.insert(i, "\n");
			x.insert(i + 1, tabs, '\t');

			// Move index.
			i += 1 + tabs;
		}
		if(x[i] == ':' && x[i + 1] == '{')
		{
			// Move '{' to new line and tabulate.
			x.insert(i + 1, "\n");
			x.insert(i + 2, tabs, '\t');

			// Move index.
			i += 1 + tabs;
		}
		if(x[i] == ':' && x[i + 1] != '{')
		{
			// Add space after ':'.
			x.insert(i + 1, " ");

			// Move index.
			i += 1;
		}
	}

	// Return string.
	return x;
}

namespace twitch
{
	auto get_vods() -> nt::sys::coro<void>
	{
		// nt::https::client client;
		// client.connect("");

		json::object variables = {{"limit", 30}, {"channelOwnerLogin", "zakvielchannel"}, {"broadcastType", "ARCHIVE"}, {"videoSort", "TIME"}};
		json::object query = {{"version", 1}, {"sha256Hash", "a937f1d22e269e39a03b509f65a7490f9fc247d7f83d6ac1421523e3b68042cb"}};

		json::array arr =
		{
			{
				{"operationName", "FilterableVideoTower_Videos"},
				{"variables", variables},
				{"extensions", { {"persistedQuery", query} }}
			}
		};

		fmt::print("{}\n\n", json::serialize(arr));
		fmt::print("{}\n\n", json_format(json::serialize(arr)));
		co_return;
	}
}

auto coro() -> nt::sys::coro<void>
{
	co_await twitch::get_vods();

	// nt::https::client client;
	// co_await client.connect("www.twitch.tv");

	// http::request<http::empty_body> request {http::verb::get, "/zakvielchannel/videos?filter=archives&sort=time", 11};
	// request.set("host", "www.twitch.tv");
	// co_await client.write(request);

	// http::response<http::string_body> response;
	// co_await client.read(response);
	// // fmt::print("{}", response);
	// std::cout << response;

	// co_await client.disconnect();
	co_return;
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
