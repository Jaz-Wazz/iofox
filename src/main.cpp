#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
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
#include <boost/system/detail/errc.hpp>
#include <boost/system/detail/error_code.hpp>
#include <exception>
#include <fmt/core.h>
#include <jaja_notation.hpp>
#include <optional>
#include <stdexcept>
#include <as_result.hpp>

namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;	// NOLINT
namespace asio = boost::asio;					// NOLINT
namespace beast = boost::beast;					// NOLINT
namespace this_coro = asio::this_coro;			// NOLINT

namespace netfox::http
{
	class client
	{
		prv std::optional<asio::ip::tcp::socket> sock;

		pbl auto connect(std::string host) -> asio::awaitable<void>
		{
			auto ret = co_await asio::ip::tcp::resolver(co_await this_coro::executor).async_resolve(host, "http", asio::use_awaitable);
			sock.emplace(co_await this_coro::executor);
			co_await asio::async_connect(sock.value(), ret, asio::use_awaitable);
		}

		pbl auto send_request(auto & request) -> asio::awaitable<void>
		{
			co_await beast::http::async_write(sock.value(), request, asio::use_awaitable);
		}

		pbl auto read_response(auto & response) -> asio::awaitable<void>
		{
			beast::flat_buffer buf;
			co_await beast::http::async_read(sock.value(), buf, response, asio::use_awaitable);
		}

		pbl auto disconnect()
		{
			sock->shutdown(sock->shutdown_both);
			sock->close();
		}
	};
}

#define netfox_try(x) BOOST_OUTCOME_CO_TRYX(x)
#define netfox_try(x) BOOST_OUTCOME_CO_TRYX(x)

namespace netfox::https
{
	class client
	{
		prv std::optional<asio::ssl::stream<asio::ip::tcp::socket>> stream;
		prv asio::ssl::context ssl_ctx {ssl_ctx.tlsv13_client};

		pbl auto connect(std::string host) -> asio::awaitable<outcome::outcome<void>> try
		{
			auto && executor = co_await this_coro::executor;
			auto ret = netfox_try(co_await asio::ip::tcp::resolver(executor).async_resolve(host, "httpsx", as_result(asio::use_awaitable)));
			stream.emplace(co_await this_coro::executor, ssl_ctx);
			netfox_try(co_await asio::async_connect(stream->next_layer(), ret, as_result(asio::use_awaitable)));
			SSL_set_tlsext_host_name(stream->native_handle(), host.c_str());
			netfox_try(co_await stream->async_handshake(stream->client, as_result(asio::use_awaitable)));
			co_return outcome::success();
		}
		catch(...) { co_return std::current_exception(); }

		pbl auto send_request(auto & request) -> asio::awaitable<outcome::outcome<void>> try
		{
			netfox_try(co_await beast::http::async_write(stream.value(), request, as_result(asio::use_awaitable)));
			co_return outcome::success();
		}
		catch(...) { co_return std::current_exception(); }

		pbl auto read_response(auto & response) -> asio::awaitable<outcome::outcome<void>> try
		{
			beast::flat_buffer buf;
			netfox_try(co_await beast::http::async_read(stream.value(), buf, response, as_result(asio::use_awaitable)));
			co_return outcome::success();
		}
		catch(...) { co_return std::current_exception(); }

		pbl auto disconnect() -> asio::awaitable<outcome::outcome<void>> try
		{
			auto ret = co_await stream->async_shutdown(as_result(asio::use_awaitable));
			if(ret.has_error() && ret.error() != asio::ssl::error::stream_truncated) co_return ret.error();
			stream->next_layer().shutdown(stream->next_layer().shutdown_both);
			stream->next_layer().close();
			fmt::print("disconnected.\n");
			co_return outcome::success();
		}
		catch(...) { co_return std::current_exception(); }
	};
}

auto coro() -> asio::awaitable<void>
{
	netfox::https::client client;
	// co_await client.send_request(req);

	auto state = co_await [&] -> asio::awaitable<outcome::outcome<void>>
	{
		netfox_try(co_await client.connect("exmaple.com"));

		beast::http::request<beast::http::empty_body> request {beast::http::verb::get, "/", 11};
		request.set("host", "exmaple.com");
		netfox_try(co_await client.send_request(request));

		beast::http::response<beast::http::string_body> response;
		netfox_try(co_await client.read_response(response));
		fmt::print("response readed: '{}'.\n", response.body());

		netfox_try(co_await client.disconnect());
		co_return outcome::success();
	}();

	// netfox::defer def = []{ on_error_code... };
	// -> ~def() -> user_async_lambd() ->

	if(state.has_failure())
	{
		fmt::print("[detected fail] - try to disconnect.\n");
		auto ret = co_await client.disconnect();
		if(!ret) fmt::print("[detected fail] - disconnect failed: ignore.\n");
		if(state.has_error()) fmt::print("[detected fail] - Fail by error: '{}'.\n", state.error().message());
		if(state.has_exception()) fmt::print("[detected fail] - Fail by exception.\n");
	}
}

int main()
{
	asio::io_context ctx;
	asio::co_spawn(ctx, coro(), [](std::exception_ptr ptr)
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
