cmake_minimum_required(VERSION 3.20)
project(HSV_Mapper)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenGL REQUIRED)

add_subdirectory("glad")

add_subdirectory("glfw-3.3.6")
include_directories("glfw-3.3.6/include")

add_subdirectory("glm")

add_executable(HSV_Mapper
        main.cpp
        glad/glad.c
        loadShader.cpp
        loadShader.h
        stb_image.h
        stb_image_write.h VertexBuffer.cpp VertexBuffer.h)

target_link_libraries(HSV_Mapper glfw ${GLFW_LIBRARIES})
target_link_libraries(HSV_Mapper glfw ${OPENGL_glu_LIBRARY} ${GLFW_LIBRARIES})
target_link_libraries(HSV_Mapper glad)