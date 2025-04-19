cmake_minimum_required(VERSION 3.31)

project(Glad)
include_directories("include")

add_library(glad glad.c glad.h khrplatform.h)
target_include_directories(glad PUBLIC)