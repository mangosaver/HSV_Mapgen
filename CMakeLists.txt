cmake_minimum_required(VERSION 3.20)
project(hsv_mapgen)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)

# GLFW build config
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Static linking
set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -static -lpthread ${CMAKE_CXX_STANDARD_LIBRARIES}")

add_subdirectory("glad")
add_subdirectory("glfw-3.3.8")
add_subdirectory("glm")

include_directories("glfw-3.3.8/include")
include_directories("include")

add_executable(hsv_mapgen
        src/main.cpp
        utils/load_shader.cpp
        utils/load_shader.h
        shaders/generated.h
        include/stb_image.h
        include/stb_image_write.h
        src/consts.h
        src/VertexBuffer.cpp
        src/VertexBuffer.h
        src/Texture.cpp
        src/Texture.h
        utils/parse_utils.cpp
        utils/parse_utils.h
        utils/print_help.cpp
        utils/print_help.h
        utils/config_utils.cpp
        utils/config_utils.h
        utils/log_utils.cpp
        utils/log_utils.h)

target_link_libraries(hsv_mapgen glfw ${GLFW_LIBRARIES})
target_link_libraries(hsv_mapgen glfw ${OPENGL_glu_LIBRARY} ${GLFW_LIBRARIES})
target_link_libraries(hsv_mapgen glad)