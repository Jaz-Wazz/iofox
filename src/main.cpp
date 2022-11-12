#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <exception>
#include <fmt/core.h>
#include <netfox.hpp>
#include <httpfox.hpp>

namespace asio = boost::asio; // NOLINT

auto coro() -> asio::awaitable<void>
{
	auto ret = co_await netfox::dns::resolve("http", "exmaple.com");
	for(auto el : ret) fmt::print("Ip: '{}'.\n", el.endpoint().address().to_string());
	co_return;
}

int main() try
{
	asio::io_context ctx;
	asio::co_spawn(ctx, coro(), asio::detached);
	return ctx.run();
} catch(std::exception & e) { fmt::print("Except: '{}'.\n", e.what()); }
