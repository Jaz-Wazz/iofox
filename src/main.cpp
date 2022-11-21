// #include <boost/asio/awaitable.hpp>
// #include <boost/asio/co_spawn.hpp>
// #include <boost/asio/detached.hpp>
// #include <boost/asio/io_context.hpp>
// #include <boost/asio/steady_timer.hpp>
// #include <boost/asio/this_coro.hpp>
// #include <boost/asio/use_awaitable.hpp>
// #include <chrono>
// #include <exception>
// #include <fmt/core.h>
// #include <netfox.hpp>
// #include <httpfox.hpp>
// #include <system_error>

// namespace asio		= boost::asio;		// NOLINT
// namespace this_coro	= asio::this_coro;	// NOLINT

// // enum class err
// // {
// // 	hostname_not_found,
// // 	internal
// // };

// // template <> struct std::is_error_code_enum<err> : true_type {};

// // auto coro() -> asio::awaitable<void> try
// // {
// // 	fmt::print("1.\n");
// // 	asio::ip::tcp::resolver resolver {co_await this_coro::executor};
// // 	fmt::print("2.\n");
// // 	auto ret = co_await resolver.async_resolve("sdfawegaweg.exe", "http", asio::use_awaitable);
// // 	fmt::print("3.\n");
// // 	for(auto el : ret) fmt::print("- ip: '{}'.\n", el.endpoint().address().to_string());
// // 	fmt::print("4.\n");
// // }
// // catch(std::exception & e) { fmt::print("Except: '{}'.\n", e.what()); }
// // catch(...) { fmt::print("Except: ''.\n"); }

// // int main() try
// // {
// // 	asio::io_context ctx;
// // 	asio::co_spawn(ctx, coro(), asio::detached);
// // 	return ctx.run();
// // } catch(std::exception & e) { fmt::print("Except: '{}'.\n", e.what()); }
// #include <boost/stacktrace.hpp>
// #include <iostream>

// void foo()
// {
// 	std::cout << boost::stacktrace::stacktrace();
// 	// for(auto el : boost::stacktrace::stacktrace()) fmt::print("-> '{}' '{}' '{}' '{}'.\n", el.address(), el.name(), el.source_file(), el.source_line());
// }

// int main()
// {
// 	foo();
// 	return 0;
// }

// #include <boost/leaf.hpp>
// #include <boost/leaf/error.hpp>
// #include <boost/leaf/result.hpp>
// #include <fmt/core.h>

// namespace leaf = boost::leaf; // NOLINT

// leaf::result<int> foo()
// {
// 	// return leaf::new_error(10);
// 	return 5;
// }

// void f(int) {}

// leaf::result<std::string> sub_foo()
// {
// 	// auto x = foo();
// 	// if(!x) return x.error();

// 	// BOOST_LEAF_AUTO(int x, foo());

// 	auto x = ({ auto var = foo(); if(!) });

// 	return "string";
// }

// int main()
// {
// 	auto x = ({ int i = 10; i--; i; });
// 	fmt::print("{}.\n", x);

// 	// if(auto x = sub_foo())
// 	// {
// 	// 	fmt::print("Succes: '{}'.\n", x.value());
// 	// }
// 	// else
// 	// {
// 	// 	fmt::print("Some error.\n");
// 	// }

// 	return 0;
// }

// #include <expected>
// #include <fmt/core.h>
// #include <string>

// #define netfox_check(foo) ({ auto ret = foo(); if(!ret) return std::unexpected(ret.error()); ret.value(); })

// std::expected<int, std::string> foo()
// {
// 	// return 5;
// 	return std::unexpected("sas");
// }

// std::expected<int, std::string> post_foo()
// {
// 	// auto ret = foo();
// 	// if(!ret) return std::unexpected(ret.error());

// 	auto ret = netfox_check(foo);

// 	return 10;
// }

// int main()
// {
// 	if(auto ret = post_foo())
// 	{
// 		fmt::print("Value: '{}'.\n", ret.value());
// 	}
// 	else
// 	{
// 		fmt::print("Error: '{}'.\n", ret.error());
// 	}
// 	return 0;
// }

// #include <boost/leaf.hpp>
// #include <boost/leaf/error.hpp>
// #include <boost/leaf/result.hpp>
// #include <fmt/core.h>

// namespace leaf = boost::leaf; // NOLINT

// leaf::result<int> foo()
// {
// 	// return 10;
// 	return leaf::new_error(18);
// }

// int main()
// {
// 	if(auto x = foo())
// 	{
// 		fmt::print("Value: '{}'.\n", x.value());
// 	}
// 	else
// 	{
// 		fmt::print("Failed.\n");
// 	}

// 	return 0;
// }

// #include <fmt/core.h>
// class except {};
// class sub_except: public except {};

// void foo() { throw sub_except(); }

// int main() try
// {
// 	foo();
// 	return 0;
// }
// catch(except & e)
// {
// 	fmt::print("sub_except.\n");
// }
// catch(...)
// {
// 	fmt::print("unknown_except.\n");
// }
