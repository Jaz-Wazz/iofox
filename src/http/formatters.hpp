#pragma once

// boost_beast
#include <boost/beast/http/write.hpp>

// fmt
#include <fmt/core.h>
#include <fmt/ostream.h>

// local_http
#include <http/containers.hpp>

template <> struct fmt::formatter<http::request_header>: fmt::ostream_formatter {};
template <> struct fmt::formatter<http::response_header>: fmt::ostream_formatter {};
template <> struct fmt::formatter<http::request>: fmt::ostream_formatter {};
template <> struct fmt::formatter<http::response>: fmt::ostream_formatter {};
