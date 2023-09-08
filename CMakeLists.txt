cmake_minimum_required(VERSION 3.20)

project(renderer VERSION 1.0.0 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (WIN32)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wunused -fsanitize=address )
    add_link_options(-fsanitize=address -fuse-ld=lld)
endif()

find_package(Vulkan REQUIRED)
find_package(fmt REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Catch2)
find_package(stb)

execute_process(
    COMMAND bash shaders/compile_shaders.sh
)

file(COPY shaders DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/src")


add_executable(renderer 
    src/main.cpp
    src/renderer/renderer.cpp 
    src/window/window.cpp
)

target_include_directories(renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src) 
target_link_libraries(renderer fmt::fmt glfw glm::glm Catch2::Catch2 vulkan stb::stb)