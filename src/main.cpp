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
#include <boost/json/parse.hpp>
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

#define pbl public:
#define prv private:

auto json_format(const auto & str) -> std::string
{
	// Copy string.
	std::string x = str;

	// Iterate each char.
	for(int i = 0, tabs = 0, text = false; i <= x.length(); i++)
	{
		// Detect text blocks.
		if(x[i] == '"') text = !text;

		// Skip format in text block.
		if(text) continue;

		// Formating.
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
		if(x[i] == ':' && (x[i + 1] == '{' || x[i + 1] == '['))
		{
			// Move '{' to new line and tabulate.
			x.insert(i + 1, "\n");
			x.insert(i + 2, tabs, '\t');

			// Move index.
			i += 1 + tabs;
		}
		if(x[i] == ':' && (x[i + 1] != '{' || x[i + 1] != '['))
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
	class video
	{
		pbl const std::string id;
		pbl const std::string date;
		pbl const std::string title;
	};

	auto get_videos(std::string channel) -> nt::sys::coro<std::vector<twitch::video>>
	{
		// Connect.
		nt::https::client client;
		co_await client.connect("gql.twitch.tv");

		// Prepare gql data.
		const json::object vars = {{"limit", 30}, {"channelOwnerLogin", channel}, {"broadcastType", "ARCHIVE"}, {"videoSort", "TIME"}};
		const json::object query = {{"version", 1}, {"sha256Hash", "a937f1d22e269e39a03b509f65a7490f9fc247d7f83d6ac1421523e3b68042cb"}};

		// Generate gql request.
		const json::array gql_request =
		{{
			{"operationName", "FilterableVideoTower_Videos"},
			{"variables", vars},
			{"extensions",{{"persistedQuery", query}}}
		}};

		// Make request.
		http::request<http::string_body> request {http::verb::post, "/gql", 11};
		request.set("Host", "gql.twitch.tv");
		request.set("Client-Id", "kimne78kx3ncx6brgo4mv6wki5h1ko");
		request.body() = json::serialize(gql_request);
		request.prepare_payload();

		// Send request.
		co_await client.write(request);

		// Recieve response.
		http::response<http::string_body> response;
		co_await client.read(response);

		// Prepare return result.
		std::vector<twitch::video> videos;

		// Parse.
		auto edges = json::parse(response.body()).at_pointer("/0/data/user/videos/edges").as_array();

		// Extract video information.
		for(auto & video : edges)
		{
			auto id		= video.at_pointer("/node/id").as_string().c_str();
			auto date	= video.at_pointer("/node/publishedAt").as_string().c_str();
			auto title	= video.at_pointer("/node/title").as_string().c_str();
			videos.push_back({id, date, title});
		}

		// Return videos.
		co_return videos;
	}
}

auto coro() -> nt::sys::coro<void>
{
	for(auto video : co_await twitch::get_videos("zakvielchannel"))
	{
		fmt::print("{:>11}{:>25}{:>60}\n", video.id, video.date, video.title);
	}
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
