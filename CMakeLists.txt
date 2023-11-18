cmake_minimum_required(VERSION 3.20)

project(renderer VERSION 1.0.0 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (WIN32)
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Release") 
        add_compile_options(-Wall -Wextra -Wpedantic -Wunused -Wconversion -O3 -std=c++20)
        add_link_options(-fuse-ld=lld)
    else()
        add_compile_options(-g -fsanitize=address -Wall -Wextra -Wpedantic -Wunused -Wconversion -O3 -std=c++20)
        add_link_options(-fuse-ld=lld -fsanitize=address )
    endif()
endif()

find_package(Vulkan REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(fmt REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Catch2 REQUIRED)
find_package(imgui REQUIRED)

if (WIN32)
    file(COPY shaders DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

else()
    execute_process(
        COMMAND bash shaders/compile_shaders.sh
    )
    file(COPY shaders DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/src")
endif()


set(LINEAR_ALGBERA_HEADERS 
    src/linear_algebra/Mat3.hpp
    src/linear_algebra/Mat4.hpp
    src/linear_algebra/Quaternion.hpp
    src/linear_algebra/Quaternion.decl.hpp
    src/linear_algebra/Vec3.decl.hpp
    src/linear_algebra/Vec3.hpp
    src/linear_algebra/Vec4.hpp
    )
    set(UTILS_HEADERS
    src/utils/BS_thread_pool.hpp
    src/utils/Panic.hpp
    src/utils/SameAsAny.hpp
    src/utils/ScopedTimer.hpp
    src/utils/Overloaded.hpp
    )
    
    add_executable(renderer 
    src/main.cpp
    src/ray-tracing/objects.cpp
    src/ray-tracing/DisneyBrdf.cpp
    src/utils/MathUtils.cpp
    
    src/renderer/renderer.cpp
    src/renderer/renderer.hpp
    src/renderer/vk_enum_string_helper.hpp
    src/window/window.hpp
    
    ${LINEAR_ALGBERA_HEADERS}
    ${UTILS_HEADERS}
    
    src/imgui/backends/imgui_impl_glfw.cpp
    src/imgui/backends/imgui_impl_glfw.h
    src/imgui/backends/imgui_impl_vulkan.cpp
    src/imgui/backends/imgui_impl_vulkan.h
)
add_executable(tests
    src/linear_algebra/tests.cpp
    src/utils/MathUtils.cpp
    ${LINEAR_ALGBERA_HEADERS}
    ${UTILS_HEADERS}
    
)


target_include_directories(renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src) 
target_link_libraries(renderer fmt::fmt glfw glm::glm Vulkan::Vulkan vulkan-headers::vulkan-headers imgui::imgui)

target_include_directories(tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src Catch2::Catch2WithMain fmt::fmt) 
target_link_libraries(tests glm::glm Catch2::Catch2WithMain fmt::fmt)