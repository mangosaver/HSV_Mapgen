cmake_minimum_required(VERSION 3.20)
project(HSV_Mapper)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)

# Static linking
set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")

add_subdirectory("glad")

add_subdirectory("glfw-3.3.6")
include_directories("glfw-3.3.6/include")

add_subdirectory("glm")

set(c_name "shaders/frag.glsl")

# Embed shader source directly into the binary
file(READ ${c_name} content HEX)
message(${content})

string(REGEX MATCHALL "([A-Fa-f0-9][A-Fa-f0-9])" SEPARATED_HEX ${content})

# Create a counter so that we only have 16 hex bytes per line
set(counter 0)
# Iterate through each of the bytes from the source file
foreach (hex IN LISTS SEPARATED_HEX)
    # Write the hex string to the line with an 0x prefix
    # and a , postfix to seperate the bytes of the file.
    string(APPEND output_c "0x${hex},")

    message("0x${hex}")

    # Increment the element counter before the newline.
    math(EXPR counter "${counter}+1")
    if (counter GREATER 16)
        # Write a newline so that all of the array initializer
        # gets spread across multiple lines.
        string(APPEND output_c "\n    ")
        set(counter 0)
    endif ()
endforeach ()

# Generate the contents that will be contained in the source file.
set(output_c "
#include \"${c_name}.h\"
uint8_t ${c_name}_data[] = {
    ${output_c}
}\;
unsigned ${c_name}_size = sizeof(${c_name}_data)\;
")

# Generate the contents that will be contained in the header file.
set(output_h "
#ifndef ${c_name}_H
#define ${c_name}_H
#include \"stdint.h\"
extern uint8_t ${c_name}_data[]\;
extern unsigned ${c_name}_size\;
#endif // ${c_name}_H
    ")

















add_executable(HSV_Mapper
        src/main.cpp
        utils/load_shader.cpp
        utils/load_shader.h
        include/stb_image.h
        include/stb_image_write.h src/VertexBuffer.cpp src/VertexBuffer.h src/consts.h src/Texture.cpp src/Texture.h utils/parse_utils.cpp utils/parse_utils.h utils/print_help.h utils/config_utils.h utils/config_utils.cpp utils/print_help.cpp utils/log_utils.cpp utils/log_utils.h)

target_link_libraries(HSV_Mapper glfw ${GLFW_LIBRARIES})
target_link_libraries(HSV_Mapper glfw ${OPENGL_glu_LIBRARY} ${GLFW_LIBRARIES})
target_link_libraries(HSV_Mapper glad)