add_executable(renderer main.cpp
    renderer/renderer.cpp 
    window/window.cpp)

target_include_directories(renderer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(renderer PRIVATE fmt)
target_include_directories(renderer PUBLIC fmt)

target_link_libraries(renderer PRIVATE glfw)
target_include_directories(renderer PUBLIC glfw)

target_link_libraries(renderer PRIVATE glm)
target_include_directories(renderer PUBLIC glm)

target_link_libraries(renderer PRIVATE vulkan)