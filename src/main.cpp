#include <boost/asio/awaitable.hpp>
#include <boost/leaf/error.hpp>
#include <boost/leaf/handle_errors.hpp>
#include <boost/leaf/result.hpp>
#include <exception>
#include <fmt/core.h>
#include <stdexcept>

namespace leaf = boost::leaf; // NOLINT

leaf::result<std::string> get_some_string()
{
	return leaf::new_error(std::string("normal_error"));
	return "normal_string";
}

void foo()
{
	throw 10;
}

int main()
{
	// if(auto x = get_some_string())
	// {
	// 	fmt::print("Normal code way, string is: '{}'.\n", x.value());
	// }
	// else
	// {
	// 	fmt::print("Bad code way, error is '?'.\n");
	// }

	// auto ret = leaf::try_handle_some([] -> leaf::result<int>
	// {
	// 	auto str = get_some_string();
	// 	if(!str) return str.error();

	// 	return 10;
	// }, [](std::string err) -> leaf::result<int>
	// {
	// 	fmt::print("Catch error as string: '{}'.\n", err);
	// 	return 20;
	// });

	// auto ret = leaf::try_handle_some([] -> leaf::result<int>
	// {
	// 	auto str = get_some_string();
	// 	if(!str) return str.error();

	// 	return 10;
	// }, [](std::string err) -> leaf::result<int>
	// {
	// 	fmt::print("Catch error as string: '{}'.\n", err);
	// 	return 20;
	// });

	leaf::try_catch([]
	{
		fmt::print("Some code.\n");
		throw std::runtime_error {"err"};
	}, [](std::exception & i) -> boost::asio::awaitable<void>
	{
		fmt::print("Except: '{}'.\n", i.what());
		co_return;
	});

	return 0;
}
