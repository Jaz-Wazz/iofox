cmake_minimum_required (VERSION "3.24.2")

# Project.
project("ia_proxy")
set(CMAKE_CXX_STANDARD "23")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Libs.
find_package("Boost" REQUIRED "system")
find_package("fmt" CONFIG REQUIRED)
find_package("OpenSSL" REQUIRED)

# src.
file(GLOB_RECURSE src "src/*")

# Target.
add_executable("ia_proxy" ${src})
target_include_directories("ia_proxy" PUBLIC "src")
set_target_properties("ia_proxy" PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
target_link_libraries("ia_proxy" PRIVATE Boost::system fmt::fmt OpenSSL::SSL)

add_compile_definitions(BOOST_ASIO_HAS_CO_AWAIT)
