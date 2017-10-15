set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-deprecated-register")

project(MONAA CXX)

cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_FLAGS "-Wall -std=c++14 -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-flto -O3 -DRELEASE")

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(Boost REQUIRED COMPONENTS
  program_options unit_test_framework iostreams)
find_package(Eigen3 REQUIRED)

include_directories(
  tgrep/
  ${PROJECT_BINARY_DIR}
  ${Boost_INCLUDE_DIRS}
  ${Eigen3_INCLUDE_DIRS})

BISON_TARGET(tre_parser
  tgrep/tre_parser.yy
  ${CMAKE_CURRENT_BINARY_DIR}/tre_parser.tab.cc)
FLEX_TARGET(tre_lexer
  tgrep/tre_lexer.l
  ${CMAKE_CURRENT_BINARY_DIR}/tre_lexer.yy.cc)
ADD_FLEX_BISON_DEPENDENCY(tre_lexer tre_parser)

## Config for Main monaa
add_executable(monaa
  tgrep/main.cc
  tgrep/intersection.cc
  tgrep/ta2za.cc
  tgrep/tre.cc
  tgrep/intermediate_tre.cc
  ${FLEX_tre_lexer_OUTPUTS}
  ${BISON_tre_parser_OUTPUTS})

target_link_libraries(monaa
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  )

target_include_directories(monaa
  PRIVATE
  .
  ${CMAKE_CURRENT_BINARY_DIR})
set_property(TARGET monaa
  PROPERTY CXX_STANDARD 14)

## Config for Test
enable_testing()

add_executable(unit_test
  tgrep/intersection.cc
  tgrep/ta2za.cc
  tgrep/tre.cc
  tgrep/intermediate_tre.cc
  test/unit_test.cc
  test/zone_automaton_test.cc
  test/ta2za_test.cc
  test/lazy_deque_test.cc
# test/word_container_test.cc
  test/ans_vec_test.cc
  test/intersection_test.cc
  test/timed_automaton_test.cc
  test/constraint_test.cc
  test/intersection_test.cc
  test/zone_automaton_test.cc
  test/sunday_skip_value_test.cc
  test/kmp_skip_value_test.cc
  test/zone_test.cc
  test/intermediate_zone_test.cc
  test/timedFJS_test.cc
  test/tre_driver_test.cc
  test/tre_test.cc
  test/intermediate_tre_test.cc
  ${FLEX_tre_lexer_OUTPUTS}
  ${BISON_tre_parser_OUTPUTS})

target_link_libraries(unit_test
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_test(NAME unit_test
  COMMAND $<TARGET_FILE:unit_test>
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
