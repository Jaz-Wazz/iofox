#pragma once
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/json/object.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include <string>
#include <vector>
#include <net_tails.hpp>

#define pbl public:
#define prv private:
#define json boost::json
#define beast boost::beast::http

namespace twitch
{
	class video
	{
		pbl const std::string id;
		pbl const std::string date;
		pbl const std::string title;
	};

	inline auto get_videos(std::string channel) -> nt::sys::coro<std::vector<twitch::video>>
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
		beast::request<beast::string_body> request {beast::verb::post, "/gql", 11};
		request.set("Host", "gql.twitch.tv");
		request.set("Client-Id", "kimne78kx3ncx6brgo4mv6wki5h1ko");
		request.body() = json::serialize(gql_request);
		request.prepare_payload();

		// Send request.
		co_await client.write(request);

		// Recieve response.
		beast::response<beast::string_body> response;
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

#undef pbl
#undef prv
#undef json
#undef beast
