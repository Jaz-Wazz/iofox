// boost_asio
#include "iofox/this_thread.hpp"
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>

// iofox
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <chrono>
#include <functional>
#include <initializer_list>
#include <iofox/coro.hpp>
#include <iofox/rethrowed.hpp>

// fmt.
#include <fmt/core.h>

// stl.
#include <stdexcept>

// catch2
#include <catch2/catch_test_macros.hpp>

#include <boost/asio/deferred.hpp>
#include <boost/asio/experimental/parallel_group.hpp>
using namespace std::chrono_literals;

template <typename CompletionToken>
struct timed_token
{
    std::chrono::milliseconds timeout;
    CompletionToken& token;
};

// Note: this is merely a function object - a lambda.
template <typename... Signatures>
struct timed_initiation
{
    template <
        typename CompletionHandler,
        typename Initiation,
        typename... InitArgs>
    void operator()(
      CompletionHandler handler,         // the generated completion handler
      std::chrono::milliseconds timeout, // the timeout specified in our completion token
      Initiation&& initiation,           // the embedded operation's initiation (e.g. async_read)
      InitArgs&&... init_args)           // the arguments passed to the embedded initiation (e.g. the async_read's buffer argument etc)
    {
        using boost::asio::experimental::make_parallel_group;

        // locate the correct executor associated with the underling operation
        // first try the associated executor of the handler. If that doesn't
        // exist, take the associated executor of the underlying async operation's handler
        // If that doesn't exist, use the default executor (system executor currently)
        auto ex = boost::asio::get_associated_executor(handler,
                                                boost::asio::get_associated_executor(initiation));

        // build a timer object and own it via a shared_ptr. This is because its
        // lifetime is shared between two asynchronous chains. Use the handler's
        // allocator in order to take advantage of the Asio recycling allocator.
        auto alloc = boost::asio::get_associated_allocator(handler);
        auto timer = std::allocate_shared<boost::asio::steady_timer>(alloc, ex, timeout);

        // launch a parallel group of asynchronous operations - one for the timer
        // wait and one for the underlying asynchronous operation (i.e. async_read)
        make_parallel_group(
                // item 0 in the group is the timer wait
                boost::asio::bind_executor(ex,
                                    [&](auto&& token)
                                    {
                                        return timer->async_wait(std::forward<decltype(token)>(token));
                                    }),
                // item 1 in the group is the underlying async operation
                boost::asio::bind_executor(ex,
                                    [&](auto&& token)
                                    {
                                        // Finally, initiate the underlying operation
                                        // passing its original arguments
                                        return boost::asio::async_initiate<decltype(token), Signatures...>(
                                                std::forward<Initiation>(initiation), token,
                                                std::forward<InitArgs>(init_args)...);
                                    })
        ).async_wait(
                // Wait for the first item in the group to complete. Upon completion
                // of the first, cancel the others.
                boost::asio::experimental::wait_for_one(),

                // The completion handler for the group
                [handler = std::move(handler), timer](
                    // an array of indexes indicating in which order the group's
                    // operations completed, whether successfully or not
                    std::array<std::size_t, 2>,

                    // The arguments are the result of concatenating
                    // the completion handler arguments of all operations in the
                    // group, in retained order:
                    // first the steady_timer::async_wait
                    std::error_code,

                    // then the underlying operation e.g. async_read(...)
                    auto... underlying_op_results // e.g. error_code, size_t
                    ) mutable
                {
                    // release all memory prior to invoking the final handler
                    timer.reset();
                    // finally, invoke the handler with the results of the
                    // underlying operation
                    std::move(handler)(std::move(underlying_op_results)...);
                });
    }
};

// Specialise the async_result primary template for our timed_token
template <typename InnerCompletionToken, typename... Signatures>
struct boost::asio::async_result<
      timed_token<InnerCompletionToken>,  // specialised on our token type
      Signatures...>
{
    // The implementation will call initiate on our template class with the
    // following arguments:
    template <typename Initiation, typename... InitArgs>
    static auto initiate(
        Initiation&& init, // This is the object that we invoke in order to
                           // actually start the packaged async operation
        timed_token<InnerCompletionToken> t, // This is the completion token that
                                             // was passed as the last argument to the
                                             // initiating function
        InitArgs&&... init_args)      // any more arguments that were passed to
                                      // the initiating function
    {
        // we will customise the initiation through our token by invoking
        // async_initiate with our own custom function object called
        // timed_initiation. We will pass it the arguments that were passed to
        // timed(). We will also forward the initiation created by the underlying
        // completion token plus all arguments destined for the underlying
        // initiation.
        return asio::async_initiate<InnerCompletionToken, Signatures...>(
                timed_initiation<Signatures...>{},
                  t.token,   // the underlying token
                  t.timeout, // our timeout argument
                std::forward<Initiation>(init),  // the underlying operation's initiation
                std::forward<InitArgs>(init_args)... // that initiation's arguments
        );
    }
};

auto coro() -> iofox::coro<void>
{
	boost::asio::steady_timer timer {co_await boost::asio::this_coro::executor, 5s};
	fmt::print("run.\n");
	auto [ec] = co_await timer.async_wait(boost::asio::as_tuple(timed_token{10000ms, boost::asio::use_awaitable}));
	fmt::print("result: '{}'.\n", ec.message());
}

TEST_CASE()
{
	iofox::this_thread::set_language("en_us");
	boost::asio::io_context io_context;
	boost::asio::co_spawn(io_context, coro(), iofox::rethrowed);
	io_context.run();
}
