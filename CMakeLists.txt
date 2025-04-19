cmake_minimum_required(VERSION 3.18)
project(hsv_mapgen CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This has not been tested
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

include(FetchContent)

# glad
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v0.1.36
)
FetchContent_MakeAvailable(glad)

# glfw
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# glm
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

# Source files
file(GLOB_RECURSE SRC_FILES
    src/*.cpp
    utils/*.cpp
)

add_executable(hsv_mapgen ${SRC_FILES})

# Find OpenGL + GLU (dynamic)
find_package(OpenGL REQUIRED)
target_include_directories(hsv_mapgen PRIVATE ${OPENGL_INCLUDE_DIRS})

include_directories(include)

target_include_directories(hsv_mapgen PRIVATE
    ${glad_SOURCE_DIR}/include
    ${glfw_SOURCE_DIR}/include
    ${glm_SOURCE_DIR}
)

target_link_libraries(hsv_mapgen
    PRIVATE
        glad
        glfw
        glm
        ${OPENGL_gl_LIBRARY}
        ${OPENGL_glu_LIBRARY}
        dl
        pthread
)
