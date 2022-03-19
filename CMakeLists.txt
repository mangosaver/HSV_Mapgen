cmake_minimum_required(VERSION 3.20)
project(HSV_Mapper)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory("glad")

add_subdirectory("glfw-3.3.6")
include_directories("glfw-3.3.6/include")

add_executable(HSV_Mapper main.cpp glad/glad.c stb_image.h stb_image_write.h)

target_link_libraries(HSV_Mapper glfw ${GLFW_LIBRARIES})
target_link_libraries(HSV_Mapper glfw ${OPENGL_glu_LIBRARY} ${GLFW_LIBRARIES})
target_link_libraries(HSV_Mapper glad)