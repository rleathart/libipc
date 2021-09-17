function(add_tests)
  foreach(test_name IN LISTS ARGV)
    add_executable(test_${test_name} ${test_name})
    target_link_libraries(test_${test_name} ${PROJECT_NAME})
    add_test(NAME ${test_name} COMMAND test_${test_name})
  endforeach()
endfunction()

add_tests(
  socket_init.c
  socket_connect.c
  socket_is_connected.c
  )
