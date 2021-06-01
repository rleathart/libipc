cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(GenerateExportHeader)

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

GENERATE_EXPORT_HEADER(${PROJECT_NAME}
  BASE_NAME ${PROJECT_NAME}
  EXPORT_MACRO_NAME IPC_EXPORT
  EXPORT_FILE_NAME ${CMAKE_CURRENT_LIST_DIR}/include/ipc/ipc_exports.h
  STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC # Is this required?
  )

add_executable(test_pipe test/pipe.c)
target_include_directories(test_pipe PUBLIC include)
target_link_libraries(test_pipe ${PROJECT_NAME})
