cmake_minimum_required(VERSION 3.19...3.28 FATAL_ERROR)

project(trim
  DESCRIPTION "Trim operator plugin for Tenzir"
  LANGUAGES CXX)

find_package(Tenzir REQUIRED PATHS "/opt/tenzir")

TenzirRegisterPlugin(
  TARGET trim
  ENTRYPOINT "src/plugin.cpp"
  SOURCES GLOB "src/*.cpp"
  INCLUDE_DIRECTORIES "include")
