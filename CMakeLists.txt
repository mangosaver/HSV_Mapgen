cmake_minimum_required(VERSION 3.20)
project(HSV_Mapper)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenGL REQUIRED)

# Static linking
set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")

add_subdirectory("glad")

add_subdirectory("glfw-3.3.6")
include_directories("glfw-3.3.6/include")

add_subdirectory("glm")

add_executable(HSV_Mapper
        src/main.cpp
        glad/glad.c
        src/loadShader.cpp
        src/loadShader.h
        src/stb_image.h
        src/stb_image_write.h src/VertexBuffer.cpp src/VertexBuffer.h src/BufferRenderer.cpp src/BufferRenderer.h src/consts.h src/Texture.cpp src/Texture.h src/utils.cpp src/utils.h)

target_link_libraries(HSV_Mapper glfw ${GLFW_LIBRARIES})
target_link_libraries(HSV_Mapper glfw ${OPENGL_glu_LIBRARY} ${GLFW_LIBRARIES})
target_link_libraries(HSV_Mapper glad)