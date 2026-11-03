add_executable(test_bytecode_viewer
  test_bytecode_viewer.cpp
)

target_link_libraries(test_bytecode_viewer PRIVATE bytecode_viewer)

add_test(NAME bytecode_viewer_test COMMAND test_bytecode_viewer
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
