set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-deprecated-register")

project(simple_wc_example CXX)

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
  ${Boost_INCLUDE_DIRS})

# BISON_TARGET(regexpr_parser
#   yare/regexpr_parser.yy
#   ${CMAKE_CURRENT_BINARY_DIR}/regexpr_parser.tab.cc)
# FLEX_TARGET(regexpr_lexer
#   yare/regexpr_lexer.l
#   ${CMAKE_CURRENT_BINARY_DIR}/regexpr_lexer.yy.cc)
# ADD_FLEX_BISON_DEPENDENCY(regexpr_lexer regexpr_parser)

# add_executable(main
#   yare/main.cc
#   yare/regexpr.cc
#   ${FLEX_regexpr_lexer_OUTPUTS}
#   ${BISON_regexpr_parser_OUTPUTS})

# target_link_libraries(main
#   ${Boost_PROGRAM_OPTIONS_LIBRARY}
#   )

# target_include_directories(main
#   PRIVATE
#   .
#   ${CMAKE_CURRENT_BINARY_DIR})
# set_property(TARGET main
#   PROPERTY CXX_STANDARD 14)

## Config for Test
enable_testing()

add_executable(unit_test
  tgrep/ta2za.cc
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
  ${FLEX_regexpr_lexer_OUTPUTS}
  ${BISON_regexpr_parser_OUTPUTS})

target_link_libraries(unit_test
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_test(NAME unit_test
  COMMAND $<TARGET_FILE:unit_test>
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
