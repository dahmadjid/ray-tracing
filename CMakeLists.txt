cmake_minimum_required(VERSION 3.20)

project(renderer VERSION 1.0.0 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (WIN32)
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Release") 
        add_compile_options(-Wall -Wextra -Wpedantic -Wunused -Wconversion -O3 -std=c++20 -ffast-math -fdiagnostics-color=always)
        add_link_options(-fuse-ld=mold)
    else()
        add_compile_options(-g -fsanitize=address -Wall -Wextra -Wpedantic -Wunused -Wconversion -O3 -std=c++20 -fdiagnostics-color=always)
        add_link_options(-fuse-ld=mold -fsanitize=address)
    endif()
endif()

find_package(Vulkan REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(fmt REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Catch2 REQUIRED)

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


set(UTILS
    src/utils/BS_thread_pool.hpp
    src/utils/Panic.hpp
    src/utils/SameAsAny.hpp
    src/utils/ScopedTimer.hpp
    src/utils/Overloaded.hpp
    src/utils/MathUtils.hpp
    src/utils/MathUtils.cpp
    src/utils/BMP.cpp
    src/utils/BMP.hpp
    src/utils/Obj.hpp
    src/utils/Obj.cpp
    
)
    
add_executable(renderer 
    src/main.cpp
    src/ray-tracing/objects.cpp
    src/ray-tracing/Scene.hpp
    src/ray-tracing/objects.hpp
    src/ray-tracing/Ray.hpp
    src/ray-tracing/Camera.hpp
    src/ray-tracing/Material.cpp
    src/ray-tracing/Material.hpp

    src/renderer/renderer.cpp
    src/renderer/renderer.hpp
    src/window/window.hpp
    
    ${LINEAR_ALGBERA_HEADERS}
    ${UTILS}
)

add_executable(tests
    src/linear_algebra/tests.cpp
    ${LINEAR_ALGBERA_HEADERS}
    ${UTILS}
)


target_include_directories(renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src) 
target_link_libraries(renderer fmt::fmt glfw glm::glm Vulkan::Vulkan vulkan-headers::vulkan-headers)

target_include_directories(tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src Catch2::Catch2WithMain fmt::fmt) 
target_link_libraries(tests glm::glm Catch2::Catch2WithMain fmt::fmt)