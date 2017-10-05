set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-deprecated-register")

project(simple_wc_example CXX)

cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-flto -O3 -DRELEASE")

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(Boost REQUIRED COMPONENTS
  PROGRAM_OPTIONS UNIT_TEST_FRAMEWORK IOSTREAMS)
find_package(PkgConfig REQUIRED)
pkg_check_modules(RE2 re2 REQUIRED)

include_directories(
  tgrep/
  ${PROJECT_BINARY_DIR}
  ${Boost_INCLUDE_DIRS}
  ${RE2_INCLUDE_DIRS})

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
#   ${RE2_LIBRARIES}
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
  test/unit_test.cc
  ${FLEX_regexpr_lexer_OUTPUTS}
  ${BISON_regexpr_parser_OUTPUTS})

target_link_libraries(unit_test
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${RE2_LIBRARIES}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_test(NAME unit_test
  COMMAND $<TARGET_FILE:unit_test>
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
