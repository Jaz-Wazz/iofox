# CMake version.
cmake_minimum_required(VERSION "3.27.4")

# Project.
project("iofox")

# Find libs.
find_package("Boost" REQUIRED "system" "json" "url")
find_package("fmt" REQUIRED)
find_package("OpenSSL" REQUIRED)

# Find sources.
file(GLOB_RECURSE src "src/*")

# [Target] - Make.
add_executable(${PROJECT_NAME} ${src})

# [Target] - Properties.
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${PROJECT_NAME} PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CMAKE_CXX_EXTENSIONS OFF)

# [Target] - Includes.
target_include_directories(${PROJECT_NAME} PUBLIC "src")

# [Target] - Linking.
target_link_libraries(${PROJECT_NAME} PRIVATE
	Boost::system
	Boost::url
	OpenSSL::SSL
	OpenSSL::Crypto
	fmt::fmt
	ws2_32
	wsock32
)

# [Target] - Macros.
target_compile_definitions("iofox" PRIVATE
	BOOST_ASIO_NO_DEPRECATED
	BOOST_ASIO_HAS_CO_AWAIT
)

# [Clang] - Enable LTO optimization.
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(LLVM_ENABLE_LTO "Thin")
endif()

# [MSVC] - Enable big objects support.
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(${PROJECT_NAME} PRIVATE "/bigobj")
endif()

# [Windows] - Set minimal api version.
if(WIN32)
	target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0601)
endif()

# Set msvc runtime library.
set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
