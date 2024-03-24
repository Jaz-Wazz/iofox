// boost_asio
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/execution/allocator.hpp>
#include <boost/asio/execution/context.hpp>
#include <boost/asio/execution_context.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/is_executor.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/query.hpp>
#include <boost/asio/require.hpp>
#include <boost/asio/require_concept.hpp>

// iofox
#include <fmt/core.h>
#include <iofox/coro.hpp>
#include <iofox/service.hpp>
#include <iofox/rethrowed.hpp>

// catch2
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>

// Define a custom property named MyCustomProperty
struct MyCustomProperty {
    static constexpr bool is_requirable_concept = true;
    static constexpr bool value() { return true; }

    template <class T>
    static constexpr bool static_query_v = T::value();

    template <class E>
    static constexpr bool require_concept(E e) {
        return e.value();
    }

    template <class T>
	static constexpr bool is_applicable_property_v = true;
};

// Example usage of the custom property
struct MyType {
    static constexpr bool value() { return true; }
};

TEST_CASE("one")
{
	boost::asio::io_context io_context;
	boost::asio::io_context::executor_type executor = io_context.get_executor();
	MyType some_my_type;
	MyCustomProperty custom_property;
	auto ret = boost::asio::require_concept(some_my_type, custom_property);


	// Check if the custom property is applicable to MyType
    bool isApplicable = boost::asio::is_applicable_property<boost::asio::io_context::executor_type, MyCustomProperty>::value;

    // Check if the custom property can be required by MyType
    bool canRequire = boost::asio::can_require_concept<MyType, MyCustomProperty>::value;
}
