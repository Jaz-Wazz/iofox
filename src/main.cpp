#include <expected>
#include <fmt/core.h>

std::expected<int, std::string> test() noexcept
{
	return 5;
	return std::unexpected("ou may its not int");
}

int main()
{
	if(auto i = test())
	{
		fmt::print("Int: '{}'.\n", i.value());
	}
	else
	{
		fmt::print("Error: '{}'.\n", i.error());
	}

	return 0;
}
