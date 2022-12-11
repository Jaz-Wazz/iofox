#pragma once
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/json/object.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include <fmt/core.h>
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

	class token
	{
		pbl const std::string value;
		pbl const std::string signature;
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

	inline auto get_token(std::string id) -> nt::sys::coro<token>
	{
		// Connect.
		nt::https::client client;
		co_await client.connect("gql.twitch.tv");

		// Prepare gql query.
		auto query =
		"query PlaybackAccessToken_Template($vodID: ID!)"
		"{videoPlaybackAccessToken(id: $vodID, params: {platform: \"web\", playerBackend: \"mediaplayer\", playerType: \"site\"})"
		"{value signature}}";

		// Generate gql request.
		json::object gql_request = {{"operationName", "PlaybackAccessToken_Template"}, {"query", query}, {"variables", {{"vodID", id}}}};

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

		// Parse.
		auto token = json::parse(response.body()).at_pointer("/data/videoPlaybackAccessToken").as_object();

		// Extract token and signature -> Pack to "twitch::token" type.
		co_return twitch::token {token.at("value").as_string().c_str(), token.at("signature").as_string().c_str()};
	}

	inline auto get_playlist(std::string id) -> nt::sys::coro<std::string>
	{
		// Connect.
		nt::https::client client;
		co_await client.connect("usher.ttvnw.net");

		// Get access token.
		auto token = co_await get_token(id);

		// Generate path with get parameters.
		auto path = fmt::format("/vod/{}.m3u8?allow_source=true&token={}&sig={}", id, token.value, token.signature);

		// Make request.
		beast::request<beast::empty_body> request {beast::verb::get, path, 11};
		request.set("Host", "usher.ttvnw.net");

		// Send request.
		co_await client.write(request);

		// Recieve response.
		beast::response<beast::string_body> response;
		co_await client.read(response);

		// Return m3u8 playlist.
		co_return response.body();
	}
}

#undef pbl
#undef prv
#undef json
#undef beast
