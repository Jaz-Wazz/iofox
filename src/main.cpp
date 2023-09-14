#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <iofox/core.hpp>
#include <iofox/windows.hpp>
#include <fmt/core.h>

namespace asio { using namespace boost::asio; }

auto coro() -> io::coro<void>
{
	io::http::request request {"POST", "/post", {{"Host", "httpbin.org"}}};
	auto response = co_await io::http::send("https://httpbin.org", request);
	response.debug_dump();
}

int main() try
{
	io::windows::set_asio_locale(io::windows::lang::english);
	asio::io_context ctx;
	asio::co_spawn(ctx, coro(), io::rethrowed);
	return ctx.run();
}
catch(const std::exception & exception)
{
	fmt::print("[main] - exception: '{}'.\n", exception.what());
}
