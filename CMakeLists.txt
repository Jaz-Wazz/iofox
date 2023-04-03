cmake_minimum_required(VERSION "3.24.2")

# Project.
project("iofox")

# Enable LTO for clang.
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(LLVM_ENABLE_LTO "Thin")
endif()

# Find libs.
find_package("Boost" REQUIRED "system")
find_package("fmt" REQUIRED)
find_package("boringssl" REQUIRED)

# Make target.
add_executable("server" "src/server.cpp" "src/precompile.cpp")
add_executable("client" "src/client.cpp" "src/precompile.cpp")

# Set target properties.
set_property(TARGET "server" "client" PROPERTY CXX_STANDARD 23)
set_property(TARGET "server" "client" PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(TARGET "server" "client" PROPERTY CMAKE_CXX_EXTENSIONS OFF)

# Add same-include support.
target_include_directories("server" PUBLIC "src")
target_include_directories("client" PUBLIC "src")

# Link target.
target_link_libraries("server" PRIVATE
	Boost::system
	fmt::fmt
	boringssl::ssl
	boringssl::crypto
)
target_link_libraries("client" PRIVATE
	Boost::system
	fmt::fmt
	boringssl::ssl
	boringssl::crypto
)

# Add macros.
target_compile_definitions("server" PRIVATE
	BOOST_ASIO_SEPARATE_COMPILATION
	BOOST_ASIO_NO_DEPRECATED
	BOOST_ASIO_NO_DYNAMIC_BUFFER_V1
	BOOST_ASIO_HAS_CO_AWAIT
)
target_compile_definitions("client" PRIVATE
	BOOST_ASIO_SEPARATE_COMPILATION
	BOOST_ASIO_NO_DEPRECATED
	BOOST_ASIO_NO_DYNAMIC_BUFFER_V1
	BOOST_ASIO_HAS_CO_AWAIT
)

# Setting minimal api version for windows.
if(WIN32)
	target_compile_definitions("server" PRIVATE _WIN32_WINNT=0x0601)
	target_compile_definitions("client" PRIVATE _WIN32_WINNT=0x0601)
endif()

# Set msvc runtime library.
set_target_properties("server" PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_target_properties("client" PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Add big objects support for msvc.
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options("server" PRIVATE "/bigobj")
	target_compile_options("client" PRIVATE "/bigobj")
endif()
