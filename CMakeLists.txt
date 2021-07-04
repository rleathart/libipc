cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (WIN32)
  add_compile_definitions(
    _CRT_SECURE_NO_WARNINGS
    _CRT_NONSTDC_NO_DEPRECATE
    )
endif()

project(ipc C)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
  add_compile_definitions(_DEBUG=1 DEBUG=1)
endif()

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS YES CACHE BOOL "Export all symbols")

add_library(${PROJECT_NAME}
  src/socket.c
  src/error.c
  )
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

if(IPC_BUILD_TESTS)
  include(CTest)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(CHECK REQUIRED check)

  include_directories(include ${CHECK_INCLUDE_DIRS})
  link_directories(${CHECK_LIBRARY_DIRS})
  add_executable(test_socket test/socket.c)
  target_link_libraries(test_socket ${PROJECT_NAME} ${CHECK_LIBRARIES})
  add_test(NAME socket COMMAND test_socket)
endif()
