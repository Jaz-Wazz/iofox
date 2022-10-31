#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>
#include <fmt/core.h>
#include <iostream>
#include <httpfox.hpp>
#include <stdexcept>
#include <utility>

namespace asio = boost::asio;					// NOLINT
namespace beast = boost::beast;					// NOLINT
namespace this_coro = boost::asio::this_coro;	// NOLINT

asio::awaitable<void> coro()
{
	try
	{
		throw std::runtime_error {"x"};
	}
	catch(std::exception e) // [std::exception] -> crash | [std::exception &] -> work.
	{
		fmt::print("b.\n");
		co_return;
	}
	catch(...)
	{
		fmt::print("a.\n");
		co_return;
	}
}

int main() try
{
    asio::io_context ctx;
	asio::co_spawn(ctx, coro(), asio::detached);
    return ctx.run();
} catch(std::exception e) { fmt::print("Except: '{}'.\n", e.what()); }
