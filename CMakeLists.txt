cmake_minimum_required(VERSION 3.16)

project(DSA LANGUAGES CXX)
include(FetchContent)

FetchContent_Declare(
  gsl
  GIT_REPOSITORY https://github.com/microsoft/GSL.git
  GIT_TAG v4.2.0)
FetchContent_MakeAvailable(gsl)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_executable(DSA main.cpp ./data_structure/linked_list/exception.cpp)
target_link_libraries(DSA PRIVATE Microsoft.GSL::GSL)

if(PROJECT_IS_TOP_LEVEL AND UNIX)
  # Create symlink to ./build/compile_commands.json for IDE to pick it up, ref:
  # https://stackoverflow.com/questions/59263015/cmake-how-to-change-compile-commands-json-output-location
  set(COMPILE_COMMANDS_SYMLINK_CMD
      ${CMAKE_COMMAND} -E create_symlink
      ${CMAKE_BINARY_DIR}/compile_commands.json
      ${CMAKE_CURRENT_SOURCE_DIR}/build/compile_commands.json)

  # Configure-time
  execute_process(COMMAND ${COMPILE_COMMANDS_SYMLINK_CMD})

  # Build-time
  add_custom_target(
    symlink_compile_commands ALL
    COMMAND ${COMPILE_COMMANDS_SYMLINK_CMD}
    COMMENT "Refreshing symlink for
   compile_commands.json")
endif()
