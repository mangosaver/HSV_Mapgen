cmake_minimum_required(VERSION 3.31)
cmake_policy(VERSION 3.31)

set(GLM_VERSION "0.9.9")
project(glm VERSION ${GLM_VERSION} LANGUAGES CXX)
enable_testing()

add_subdirectory(glm)
add_library(glm::glm ALIAS glm)