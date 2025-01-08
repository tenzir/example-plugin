cmake_minimum_required(VERSION 3.19...3.28 FATAL_ERROR)

project(example
  DESCRIPTION "Example plugin for Tenzir"
  LANGUAGES CXX)

find_package(Tenzir REQUIRED PATHS "/opt/tenzir")

# This plugin is set up in a way where every builtin/*.cpp file may define its
# own plugin. The main src/plugin.cpp file is just a stub that acts as a parent
# to all the other plugins.
TenzirRegisterPlugin(
  TARGET example
  ENTRYPOINT "src/plugin.cpp"
  SOURCES GLOB "src/*.cpp"
  BUILTINS GLOB "builtins/*.cpp"
  INCLUDE_DIRECTORIES "include")
