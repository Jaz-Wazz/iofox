#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/file.hpp>
#include <boost/beast/core/file_base.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/system/detail/error_code.hpp>
#include <fmt/core.h>
#include <iofox.hpp>
#include <twitch.hpp>
#include <util.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace asio = boost::asio;			// NOLINT.
namespace beast = boost::beast;			// NOLINT.
namespace http = beast::http;			// NOLINT.
namespace this_coro = asio::this_coro;	// NOLINT.

auto coro() -> io::coro<void>
{
	// io::http::client client;
	// co_await client.connect("https://adbtc.top");

	// io::http::request<> request {"GET", "/", {{"host", "adbtc.top"}}};
	// co_await client.write(request);

	// io::http::response<std::vector<char>> response;
	// co_await client.read(response);
	// std::cout << response << '\n';
	co_return;
}

int main() try
{
	// empty.
	{
		io::http::request request {"POST", "/", {{"sas", "sis"}, {"key", "val"}}};
		std::cout << request << '\n';
	}

	// string.
	{
		io::http::request<std::string> request {"POST", "/", {{"sas", "sis"}, {"key", "val"}}, "string"};
		std::cout << request << '\n';
	}
	{
		std::string s = "copy string";
		io::http::request<std::string> request {"POST", "/", {{"sas", "sis"}, {"key", "val"}}, s};
		std::cout << request << '\n';
	}
	{
		std::string s = "mooved string";
		io::http::request<std::string> request {"POST", "/", {{"sas", "sis"}, {"key", "val"}}, std::move(s)};
		std::cout << request << '\n';
	}

	// vector.
	{
		io::http::request<std::vector<char>> request {"POST", "/", {{"sas", "sis"}, {"key", "val"}}, {'b', 'o', 'd', 'y'}};
		std::cout << request << '\n';
	}
	{
		std::vector<char> body = {'b', 'o', 'd', 'y'};
		io::http::request<std::vector<char>> request {"POST", "/", {{"sas", "sis"}, {"key", "val"}}, body};
		std::cout << request << '\n';
	}
	{
		std::vector<char> body = {'b', 'o', 'd', 'y'};
		io::http::request<std::vector<char>> request {"POST", "/", {{"sas", "sis"}, {"key", "val"}}, std::move(body)};
		std::cout << request << '\n';
	}

	// file.
	{
		io::http::request<io::file> request {"POST", "/", {{"sas", "sis"}, {"key", "val"}}, {"sas.txt"}};
	}
	{
		io::file file {"sas_2.txt"};
		io::http::request<io::file> request {"POST", "/", {{"sas", "sis"}, {"key", "val"}}, std::move(file)};
	}
	{
		beast::file file;
		beast::error_code e;
		file.open("sas_3.txt", beast::file_mode::write, e);
		io::http::request<beast::file> request {"POST", "/", {{"sas", "sis"}, {"key", "val"}}, std::move(file)};
	}

	// io::windows::set_asio_locale(io::windows::lang::english);
	// asio::io_context ctx;
	// asio::co_spawn(ctx, coro(), io::rethrowed);
	// return ctx.run();
}
catch(std::exception & e) { fmt::print("Exception: '{}'.\n", e.what()); }
catch(...) { fmt::print("Exception: 'unknown'.\n"); }
