#include <chrono>
#include <cstdio>
#include <fmt/core.h>
#include <exception>
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <fmt/ranges.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>

auto generate_data_chunk_a() -> std::string
{
	std::string string;
	string.reserve(528888890);
	for(int i : std::views::iota(0, 60'000'000))
	{
		string += fmt::format_int(i).c_str();
		string += ' ';
	}
	return string;
}

auto generate_data_chunk_b() -> std::string
{
	fmt::memory_buffer buffer;
	buffer.reserve(528888890);
	fmt::format_to(std::back_inserter(buffer), "{}", fmt::join(std::views::iota(0, 60'000'000), " "));
	return "";
}

auto generate_data_chunk_c() -> std::string
{
    std::string string;
	string.reserve(528888890);
    fmt::format_to(std::back_inserter(string), "{}", fmt::join(std::views::iota(0, 60'000'000), " "));
    return string;
}

auto generate_data_chunk_d() -> std::string
{
    return fmt::format("{}", fmt::join(std::views::iota(0, 60'000'000), " "));
}

auto generate_data_chunk_e() -> std::string
{
	std::string string;
	string.reserve(528888890);

	for(int i : std::views::iota(0, 60'000'000))
	{
		fmt::format_to(std::back_inserter(string), "{} ", i);
	}
	return string;
}

auto generate_data_chunk_f() -> std::string
{
	std::string string;
	string.reserve(528888890);
	std::stringstream stream {std::move(string)};
	for(int i : std::views::iota(0, 60'000'000))
	{
		fmt::format_to(std::ostreambuf_iterator<char>(stream), "{} ", i);
	}
	return std::move(stream.str());
}

auto generate_data_chunk_g() -> std::string
{
	std::stringstream stream;
	for(int i : std::views::iota(0, 60'000'000))
	{
		fmt::format_to(std::ostreambuf_iterator<char>(stream), "{} ", i);
	}
	return std::move(stream.str());
}

auto generate_data_chunk_h() -> std::string
{
	std::string string;
	string.reserve(528888890);
	std::stringstream stream {std::move(string)};
	for(int i : std::views::iota(0, 60'000'000))
	{
		stream << i << ' ';
	}
	return std::move(stream.str());
}

void benchmark(auto title, auto callable)
{
	auto start = std::chrono::steady_clock::now();
	std::string chunk = callable();
	auto finish = std::chrono::steady_clock::now();
	fmt::print("Time '{}': {}.\n", title, std::chrono::duration_cast<std::chrono::milliseconds>(finish - start));
	// std::ofstream(fmt::format("{}.txt", title)) << chunk;
}

int main() try
{
	benchmark("generate_data_chunk_a", generate_data_chunk_a);
	benchmark("generate_data_chunk_b", generate_data_chunk_b);
	benchmark("generate_data_chunk_c", generate_data_chunk_c);
	benchmark("generate_data_chunk_d", generate_data_chunk_d);
	benchmark("generate_data_chunk_e", generate_data_chunk_e);
	benchmark("generate_data_chunk_f", generate_data_chunk_f);
	benchmark("generate_data_chunk_g", generate_data_chunk_g);
	benchmark("generate_data_chunk_h", generate_data_chunk_h);
	return 0;
}
catch(const std::exception & e)
{
	fmt::print("Exception: '{}'.\n", e.what());
}
