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
#include <boost/outcome/result.hpp>
#include <exception>
#include <fmt/core.h>
#include <jaja_notation.hpp>
#include <optional>
#include <stdexcept>

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

namespace netfox::https
{
	class client
	{
		prv std::optional<asio::ssl::stream<asio::ip::tcp::socket>> stream;
		prv asio::ssl::context ssl_ctx {ssl_ctx.tlsv13_client};

		pbl auto connect(std::string host) -> asio::awaitable<void>
		{
			auto ret = co_await asio::ip::tcp::resolver(co_await this_coro::executor).async_resolve(host, "https", asio::use_awaitable);
			stream.emplace(co_await this_coro::executor, ssl_ctx);
			co_await asio::async_connect(stream->next_layer(), ret, asio::use_awaitable);
			SSL_set_tlsext_host_name(stream->native_handle(), host.c_str());
			co_await stream->async_handshake(stream->client, asio::use_awaitable);
		}

		pbl auto send_request(auto & request) -> asio::awaitable<void>
		{
			co_await beast::http::async_write(stream.value(), request, asio::use_awaitable);
		}

		pbl auto read_response(auto & response) -> asio::awaitable<void>
		{
			beast::flat_buffer buf;
			co_await beast::http::async_read(stream.value(), buf, response, asio::use_awaitable);
			throw std::runtime_error("sas");
		}

		pbl auto disconnect() -> asio::awaitable<void>
		{
			auto [err] = co_await stream->async_shutdown(asio::as_tuple(asio::use_awaitable));
			if(err != asio::ssl::error::stream_truncated) throw err;
			stream->next_layer().shutdown(stream->next_layer().shutdown_both);
			stream->next_layer().close();
			fmt::print("disconnected.\n");
		}
	};
}

auto coro() -> asio::awaitable<void>
{
	std::function<asio::awaitable<void>()> on_error;
	netfox::https::client client;

	try
	{
		co_await client.connect("exmaple.com");

		beast::http::request<beast::http::empty_body> request {beast::http::verb::get, "/", 11};
		request.set("host", "exmaple.com");
		co_await client.send_request(request);

		beast::http::response<beast::http::string_body> response;
		co_await client.read_response(response);
		fmt::print("response readed: '{}'.\n", response.body());
	}
	catch(std::exception & e)
	{
		fmt::print("Inner exception: '{}'.\n", e.what());

		on_error = [&] -> asio::awaitable<void>
		{
			co_await client.disconnect();
		};
	}
	catch(...)
	{
		fmt::print("Inner exception: 'unknown'.\n");
	}

	if(on_error) co_await on_error();
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
