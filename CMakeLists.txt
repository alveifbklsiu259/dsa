cmake_minimum_required(VERSION 3.28)

project(DSA LANGUAGES CXX)

# include(FetchContent) FetchContent_Declare( gsl GIT_REPOSITORY
# https://github.com/microsoft/GSL.git GIT_TAG v4.2.0)
# FetchContent_MakeAvailable(gsl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(Catch2 3 CONFIG REQUIRED)

# --- modules ---
add_library(dsa_modules OBJECT)

target_sources(dsa_modules PRIVATE # implementations
)

target_sources(dsa_modules PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
                                  ./data_structure/tree/trie.cppm)

target_link_libraries(dsa_modules PRIVATE Microsoft.GSL::GSL)
# --- modules ---

# --- main executable ---
add_executable(DSA main.cpp)

target_link_libraries(DSA PRIVATE dsa_modules Microsoft.GSL::GSL)
# --- main executable ---

# --- Tests ---
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*_test.cpp"
     "${CMAKE_SOURCE_DIR}/src/*_test.cpp" "${CMAKE_SOURCE_DIR}/*_test.cpp")

# --- test modules ---
add_library(test_modules OBJECT)

target_sources(test_modules PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES
                                   ./tests/helper/pmr_trackers.cppm)
target_link_libraries(test_modules PRIVATE Microsoft.GSL::GSL)

# --- test modules ---
# Add a single test executable that compiles all test files
if(TEST_SOURCES)
  add_executable(unit_tests ${TEST_SOURCES})

  target_link_libraries(
    unit_tests PRIVATE dsa_modules test_modules Catch2::Catch2WithMain
                       Microsoft.GSL::GSL)

  include(CTest)
  include(Catch)
  catch_discover_tests(unit_tests)
endif()
# --- Tests ---

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
    COMMENT "Refreshing symlink for compile_commands.json")
endif()
