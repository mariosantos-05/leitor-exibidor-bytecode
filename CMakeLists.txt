cmake_minimum_required(VERSION 3.16)
project(BytecodeViewer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(BUILD_TESTING "Enable building tests" ON)
option(BUILD_DOCS "Enable building docs (Doxygen)" ON)

include(CTest)

add_subdirectory(src)

if(BUILD_DOCS AND EXISTS ${CMAKE_SOURCE_DIR}/doxygen)
  add_subdirectory(doxygen)
endif()

if(BUILD_TESTING AND EXISTS ${CMAKE_SOURCE_DIR}/tests)
  add_subdirectory(tests)
endif()

install(DIRECTORY include/ DESTINATION include)

install(EXPORT bytecode_viewerTargets FILE bytecode_viewerTargets.cmake DESTINATION lib/cmake/bytecode_viewer)
