# CMAKE generated file: DO NOT EDIT!
# Generated by CMake Version 3.30
cmake_policy(SET CMP0009 NEW)

# SRC at CMakeLists.txt:39 (file)
file(GLOB_RECURSE NEW_GLOB LIST_DIRECTORIES false "/home/dry/Documents/Eggo/Tix/./main.cpp")
set(OLD_GLOB
  "/home/dry/Documents/Eggo/Tix/./main.cpp"
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "/home/dry/Documents/Eggo/Tix/binary/CMakeFiles/cmake.verify_globs")
endif()

# SRC at CMakeLists.txt:39 (file)
file(GLOB_RECURSE NEW_GLOB LIST_DIRECTORIES false "/home/dry/Documents/Eggo/Tix/./src/*.cpp")
set(OLD_GLOB
  "/home/dry/Documents/Eggo/Tix/./src/backend/generator.cpp"
  "/home/dry/Documents/Eggo/Tix/./src/tix.cpp"
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "/home/dry/Documents/Eggo/Tix/binary/CMakeFiles/cmake.verify_globs")
endif()

# SRC at CMakeLists.txt:39 (file)
file(GLOB_RECURSE NEW_GLOB LIST_DIRECTORIES false "/home/dry/Documents/Eggo/Tix/./std/*.cpp")
set(OLD_GLOB
  "/home/dry/Documents/Eggo/Tix/./std/tix_rt.cpp"
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "/home/dry/Documents/Eggo/Tix/binary/CMakeFiles/cmake.verify_globs")
endif()
