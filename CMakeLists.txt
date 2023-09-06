cmake_minimum_required(VERSION "3.24.2")

# Project.
project("iofox")

# Enable LTO for clang.
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(LLVM_ENABLE_LTO "Thin")
endif()

# Find libs.
find_package("Boost" REQUIRED "system" "json" "url")
find_package("fmt" REQUIRED)
find_package("OpenSSL" REQUIRED)

# Find sources.
file(GLOB_RECURSE src "src/*")

# Make target.
add_executable("iofox" ${src})

# Set target properties.
set_property(TARGET "iofox" PROPERTY CXX_STANDARD 23)
set_property(TARGET "iofox" PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(TARGET "iofox" PROPERTY CMAKE_CXX_EXTENSIONS OFF)

# Add same-include support.
target_include_directories("iofox" PUBLIC "src")

# Link target.
target_link_libraries("iofox" PRIVATE
	Boost::system
	Boost::json
	Boost::url
	fmt::fmt
	OpenSSL::SSL
	OpenSSL::Crypto
	ws2_32
	wsock32
)

# Add macros.
target_compile_definitions("iofox" PRIVATE BOOST_ASIO_HAS_CO_AWAIT)

# Setting minimal api version for windows.
if(WIN32)
	target_compile_definitions("iofox" PRIVATE _WIN32_WINNT=0x0601)
endif()

# Set msvc runtime library.
set_target_properties("iofox" PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Add big objects support for msvc.
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options("iofox" PRIVATE "/bigobj")
endif()
