cmake_minimum_required(VERSION "3.31.2")
project("iofox")

file(GLOB_RECURSE src "test/*")
add_executable("test" ${src})

set_property(TARGET "test" PROPERTY CXX_STANDARD 23)
set_property(TARGET "test" PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(TARGET "test" PROPERTY CMAKE_CXX_EXTENSIONS OFF)

include("packages/boost.cmake")
include("packages/wolfssl.cmake")
include("packages/fmt.cmake")

target_link_libraries("test" PRIVATE
	Boost::asio
	Boost::beast
	Boost::url
	wolfssl::wolfssl
	fmt::fmt
	-static
)

target_include_directories("test" PUBLIC "test" "src")
