#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/outcome/outcome.hpp>
#include <boost/outcome/result.hpp>
#include <boost/outcome/success_failure.hpp>
#include <boost/outcome/try.hpp>
#include <boost/system/detail/error_code.hpp>
#include <exception>
#include <fmt/core.h>
#include <jaja_notation.hpp>
#include <optional>
#include <stdexcept>
#include <as_result.hpp>
#include <utility>

namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;	// NOLINT
namespace asio = boost::asio;					// NOLINT
namespace beast = boost::beast;					// NOLINT
namespace this_coro = asio::this_coro;			// NOLINT

namespace netfox
{
	template <typename T> using result = asio::awaitable<outcome::outcome<T>>;
	auto use_result() { return as_result(asio::use_awaitable); }
}

#define co_try(x) BOOST_OUTCOME_CO_TRYX(x)

namespace netfox::https
{
	class client
	{
		prv std::optional<asio::ssl::stream<asio::ip::tcp::socket>> stream;
		prv asio::ssl::context ssl_ctx {ssl_ctx.tlsv13_client};

		pbl auto connect(std::string host) -> netfox::result<void>
		{
			// Resolve hosts.
			auto hosts = co_await asio::ip::tcp::resolver(co_await this_coro::executor).async_resolve(host, "https", netfox::use_result());
			if(!hosts) co_return hosts.error();

			// Create ssl context.
			stream.emplace(co_await this_coro::executor, ssl_ctx);

			// Connect.
			auto connected = co_await asio::async_connect(stream->next_layer(), hosts.value(), netfox::use_result());
			if(!connected) co_return connected.error();

			// Set SNI hostname. (FIXME: Check ssl error)
			SSL_set_tlsext_host_name(stream->native_handle(), host.c_str());

			// Handshake.
			auto handshaked = co_await stream->async_handshake(stream->client, netfox::use_result());
			if(!handshaked) co_return handshaked.error();

			// Exit.
			co_return outcome::success();
		}

		pbl auto connectx(std::string host) -> netfox::result<void>
		{
			auto hosts = co_try (co_await asio::ip::tcp::resolver(co_await this_coro::executor).async_resolve(host, "https", netfox::use_result()));
			stream.emplace(co_await this_coro::executor, ssl_ctx);
			co_try (co_await asio::async_connect(stream->next_layer(), hosts, netfox::use_result()));
			SSL_set_tlsext_host_name(stream->native_handle(), host.c_str());
			co_try (co_await stream->async_handshake(stream->client, netfox::use_result()));
			co_return outcome::success();
		}

		pbl auto send_request(auto & request) -> netfox::result<void>
		{
			co_try (co_await beast::http::async_write(stream.value(), request, netfox::use_result()));
			co_return outcome::success();
		}

		pbl auto read_response(auto & response) -> netfox::result<void>
		{
			beast::flat_buffer buf;
			co_try (co_await beast::http::async_read(stream.value(), buf, response, netfox::use_result()));
			co_return outcome::success();
		}

		// pbl auto disconnect(auto && handler)
		// {
		// 	stream->async_shutdown([&](boost::system::error_code err)
		// 	{
		// 		fmt::print("disconnected.\n");
		// 		stream->next_layer().shutdown(stream->next_layer().shutdown_both);
		// 		stream->next_layer().close();
		// 		if(err == asio::ssl::error::stream_truncated) err.clear();
		// 		handler(err);
		// 	});
		// }
	};
}

auto coro() -> asio::awaitable<void>
{
	netfox::https::client client;

	auto get_page = [&] -> netfox::result<void>
	{
		// Connect.
		co_try (co_await client.connectx("exmaple.com"));

		// Request.
		beast::http::request<beast::http::empty_body> request {beast::http::verb::get, "/", 11};
		request.set("host", "exmaple.com");
		co_try (co_await client.send_request(request));

		// Response.
		beast::http::response<beast::http::string_body> response;
		co_try (co_await client.read_response(response));

		// Print.
		fmt::print("page loaded, yay! response: '{}'.\n", response.body());

		// Exit.
		co_return outcome::success();
	};

	if(auto result = co_await get_page())
	{

	}
}

auto coro_another() -> asio::awaitable<void>
{
	netfox::https::client client;
	if(co_await client.connectx("exmaple.com"))
	{
		beast::http::request<beast::http::empty_body> request {beast::http::verb::get, "/", 11};
		request.set("host", "exmaple.com");
		if(co_await client.send_request(request))
		{
			beast::http::response<beast::http::string_body> response;
			if(co_await client.read_response(response))
			{
				fmt::print("page loaded, yay! response: '{}'.\n", response.body());
			}
			else
			{
				fmt::print("Read error.\n");
				// try disconnect.
			}
		}
		else
		{
			fmt::print("Send error.\n");
			// try disconnect.
		}
	}
	else
	{
		fmt::print("Connection error.\n");
	}
}

auto coro_another_yet() -> asio::awaitable<void>
{
	netfox::https::client client;

	if(auto ret =! co_await client.connectx("exmaple.com"))
	{
		fmt::print("Connection error.\n");
	}

	beast::http::request<beast::http::empty_body> request {beast::http::verb::get, "/", 11};
	request.set("host", "exmaple.com");
	if(auto ret =! co_await client.send_request(request))
	{
		fmt::print("Send error.\n");
		// try disconnect.
	}

	beast::http::response<beast::http::string_body> response;
	if(auto ret = co_await client.read_response(response))
	{
		fmt::print("page loaded, yay! response: '{}'.\n", response.body());
	}
	else
	{
		fmt::print("Read error.\n");
		// try disconnect.
	}
}

int main()
{
	asio::io_context ctx;
	asio::co_spawn(ctx, coro_another_yet(), [](std::exception_ptr ptr)
	{
		if(ptr)
		{
			try { std::rethrow_exception(ptr); }
			catch(std::exception & e) { fmt::print("Exception: '{}'.\n", e.what()); }
			catch(...) { fmt::print("Exception: 'unknown'.\n"); }
		}
	});
	return ctx.run();
}
