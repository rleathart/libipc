cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (WIN32)
  add_compile_definitions(
    _CRT_SECURE_NO_WARNINGS
    _CRT_NONSTDC_NO_DEPRECATE
    )
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS YES CACHE BOOL "Export all symbols")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
  add_compile_definitions(_DEBUG=1 DEBUG=1)
endif()

# Just dump everything in the build dir so the executables can find dlls
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

project(ipc C)

add_subdirectory(src)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)
install(
  TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Config
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  )
install(DIRECTORY include/ DESTINATION include)
install(EXPORT ${PROJECT_NAME}Config DESTINATION lib/cmake/${PROJECT_NAME})
export(TARGETS ${PROJECT_NAME} FILE ${PROJECT_NAME}Config.cmake)

if(BUILD_TESTING AND NOT IPC_BUILD_TESTS EQUAL 0)
  enable_testing()
  add_subdirectory(test)
endif()
