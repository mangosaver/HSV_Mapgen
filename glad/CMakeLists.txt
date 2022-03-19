cmake_minimum_required(VERSION 3.0)

project(Glad)

add_library(glad glad.c glad.h)
target_include_directories(glad PUBLIC)