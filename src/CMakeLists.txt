add_library(bytecode_viewer STATIC
  bytecode_viewer/ConstantPool.cpp
  bytecode_viewer/BytecodeDecoder.cpp
  bytecode_viewer/ClassReader.cpp
)

target_include_directories(bytecode_viewer PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

add_executable(bytecode_viewer_cli
  cli/main.cpp
)
target_link_libraries(bytecode_viewer_cli PRIVATE bytecode_viewer)

install(TARGETS bytecode_viewer EXPORT bytecode_viewerTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib)
install(TARGETS bytecode_viewer_cli RUNTIME DESTINATION bin)
