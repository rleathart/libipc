cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (WIN32)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
  set(BUILD_SHARED_LIBS TRUE)
endif()

project(ipc C)

include(CTest)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CHECK REQUIRED check)

include_directories(include ${CHECK_INCLUDE_DIRS})
link_directories(${CHECK_LIBRARY_DIRS})

add_library(${PROJECT_NAME} SHARED
  src/socket.c
  src/error.c
  )

if (WIN32)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(test_socket test/socket.c)
target_link_libraries(test_socket ${PROJECT_NAME} ${CHECK_LIBRARIES})
add_test(NAME socket COMMAND test_socket)
