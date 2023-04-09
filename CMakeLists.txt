cmake_minimum_required(VERSION 3.20)

project(renderer VERSION 1.0.0 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (WIN32)
else()
# add_compile_options(-Wall -Wextra -Wpedantic -Wunused -fsanitize=address )
# add_link_options(-fsanitize=address)
set(CMAKE_CXX_FLAGS "-fuse-ld=lld")
endif()

find_package (Vulkan REQUIRED)

add_subdirectory(libs)
add_subdirectory(src)
file(COPY shaders DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/src")