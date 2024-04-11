cmake_minimum_required(VERSION 3.19...3.28 FATAL_ERROR)

project(example
  DESCRIPTION "Example plugin for Tenzir"
  LANGUAGES CXX)

find_package(Tenzir REQUIRED PATHS "/opt/tenzir")

TenzirRegisterPlugin(
  TARGET example
  ENTRYPOINT "src/plugin.cpp"
  SOURCES GLOB "src/*.cpp"
  INCLUDE_DIRECTORIES "include")
