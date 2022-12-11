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
