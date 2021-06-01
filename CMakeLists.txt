cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (WIN32)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
  set(BUILD_SHARED_LIBS TRUE)
endif()

project(ipc C)

add_library(${PROJECT_NAME} SHARED
  src/pipe.c
  src/error.c
  )

if (WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if (DOTNET_PIPES)
  add_compile_definitions(DOTNET_PIPES)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC include)

add_executable(test_pipe test/pipe.c)
target_include_directories(test_pipe PUBLIC include)
target_link_libraries(test_pipe ${PROJECT_NAME})
