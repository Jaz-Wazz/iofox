#include <coroutine>
#include <fmt/core.h>
#include <iostream>
#include <string>
#include <variant>

template <typename Result, typename Error> class Expected {
public:
  struct promise_type {
    Expected *expected;
    Expected get_return_object() { return {*this}; }
    void return_value(Expected result) { *expected = std::move(result); }

    std::suspend_never initial_suspend() const noexcept { return {}; }
    std::suspend_never final_suspend() const noexcept { return {}; }

    void unhandled_exception() noexcept {}
  };

  struct Awaiter {
    Expected expected;

    bool await_ready() { return expected.is_result(); }
    Result await_resume() { return expected.result(); }
    void await_suspend(std::coroutine_handle<promise_type> handle) {
      *handle.promise().expected = expected;
      handle.destroy();
    }
  };

  template <typename T> Expected(T t) : m_value{std::move(t)} {}

  bool is_result() const { return m_value.index() == 1; }
  Result result() const { return *std::get_if<1>(&m_value); }

  bool is_error() const { return m_value.index() == 2; }
  Error error() const { return *std::get_if<2>(&m_value); }

  template <typename T> Expected &operator=(T t) {
    m_value = std::move(t);
    return *this;
  }

  Awaiter operator co_await() { return Awaiter{*this}; }

private:
  Expected(promise_type &promise) noexcept { promise.expected = this; }

private:
  std::variant<std::monostate, Result, Error> m_value;
};

auto some_foo() -> Expected<int, char>
{
	// co_return 'x';
	co_return 10;
}

auto foo() -> Expected<int, char>
{
	auto var = co_await some_foo();
	fmt::print("Succes: '{}'.\n", var);
	co_return 5;

	// co_return 5;
	// co_return 'a';
}

int main()
{
	auto ret = foo();

	if(ret.is_result())
	{
		fmt::print("Result: '{}'.\n", ret.result());
	}
	if(ret.is_error())
	{
		fmt::print("Error: '{}'.\n", ret.error());
	}

	return 0;
}
