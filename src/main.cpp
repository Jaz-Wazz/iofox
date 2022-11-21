#include <fmt/core.h>
#include <stacktrace>

void print_stack(std::stacktrace && trace)
{
	fmt::print("Stack:\n");
	for(auto el : trace)
	{
		auto && foo = [&]
		{
			auto && desk = el.description();
			desk = desk.substr(desk.find_first_of('!') + 1);
			desk = desk.erase(desk.find_last_of('+'));
			return desk;
		}();

		auto && file = [&]
		{
			auto && path = el.source_file();
			if(path.empty()) return std::string("-");
			return path.substr(path.find_last_of('\\') + 1);
		}();

		fmt::print("{:5} -> {:30} -> {}\n", el.source_line(), foo, file);
	}
}

void bar()
{
	print_stack(std::stacktrace::current());
}

void foo()
{
	bar();
}

int main()
{
	foo();
	return 0;
}
