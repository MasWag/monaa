set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-deprecated-register")

project(MONAA CXX)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_SOURCE_DIR}/cmake/Modules/")

cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_FLAGS "-Wall -std=c++14 -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-flto -O3 -DRELEASE")

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(Boost REQUIRED COMPONENTS
  program_options unit_test_framework iostreams graph)
find_package(Eigen3 REQUIRED)
find_package(PPL REQUIRED)

find_path(GMP_INCLUDE_DIRS NAMES gmp.h)
find_library(GMP_LIBRARY NAMES gmp libgmp)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMP DEFAULT_MSG GMP_INCLUDE_DIRS GMP_LIBRARY)

include_directories(
  monaa/
  libmonaa/
  ${PROJECT_BINARY_DIR}
  ${Boost_INCLUDE_DIRS}
  ${Eigen3_INCLUDE_DIRS})

BISON_TARGET(tre_parser
  monaa/tre_parser.yy
  ${CMAKE_CURRENT_BINARY_DIR}/tre/tre_parser.tab.cc)
FLEX_TARGET(tre_lexer
  monaa/tre_lexer.l
  ${CMAKE_CURRENT_BINARY_DIR}/tre/tre_lexer.yy.cc)
ADD_FLEX_BISON_DEPENDENCY(tre_lexer tre_parser)


BISON_TARGET(constraint_parser
  monaa/constraint_parser.yy
  ${CMAKE_CURRENT_BINARY_DIR}/constraint/constraint_parser.tab.cc)
FLEX_TARGET(constraint_lexer
  monaa/constraint_lexer.l
  ${CMAKE_CURRENT_BINARY_DIR}/constraint/constraint_lexer.yy.cc)
ADD_FLEX_BISON_DEPENDENCY(constraint_lexer constraint_parser)

## Config for Main monaa
add_executable(monaa
  monaa/main.cc
  libmonaa/intersection.cc
  libmonaa/ta2za.cc
  monaa/tre.cc
  monaa/intermediate_tre.cc
  ${FLEX_tre_lexer_OUTPUTS}
  ${BISON_tre_parser_OUTPUTS})

target_link_libraries(monaa
#  profiler
  ${PPL_PPL_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_GRAPH_LIBRARY})

add_executable(pmonaa
  monaa/pmain.cc
  ${FLEX_tre_lexer_OUTPUTS}
  ${FLEX_constraint_lexer_OUTPUTS}
  ${BISON_constraint_parser_OUTPUTS})

target_link_libraries(pmonaa
#  profiler
  ${PPL_PPL_LIBRARY}
  ${GMP_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_GRAPH_LIBRARY})

target_include_directories(monaa
  PRIVATE
  .
  ${CMAKE_CURRENT_BINARY_DIR})
set_property(TARGET monaa
  PROPERTY CXX_STANDARD 14)

## Config for libmonaa
add_library(libmonaa STATIC
  libmonaa/intersection.cc
  libmonaa/ta2za.cc
)

## Config for Test
enable_testing()

add_executable(unit_test
  libmonaa/parametric_intersection.cc
  libmonaa/intersection.cc
  libmonaa/ta2za.cc
  libmonaa/pta2pza.cc
  monaa/tre.cc
  monaa/intermediate_tre.cc
  test/unit_test.cc
  test/zone_automaton_test.cc
  test/ta2za_test.cc
  test/lazy_deque_test.cc
# test/word_container_test.cc
  test/ans_vec_test.cc
  test/intersection_test.cc
  test/timed_automaton_test.cc
  test/constraint_test.cc
  test/parametric_intersection_test.cc
  test/zone_automaton_test.cc
  test/sunday_skip_value_test.cc
  test/kmp_skip_value_test.cc
  test/parametric_kmp_skip_value_test.cc
  test/zone_test.cc
  test/intermediate_zone_test.cc
  test/timedFJS_test.cc
  test/parametric_timedFJS_test.cc
  test/tre_driver_test.cc
  test/tre_test.cc
  test/intermediate_tre_test.cc
  test/interval_test.cc
  test/timed_automaton_parser_test.cc
  test/parametric_timed_automaton_parser_test.cc
  ${FLEX_tre_lexer_OUTPUTS}
  ${BISON_tre_parser_OUTPUTS}
  ${FLEX_constraint_lexer_OUTPUTS}
  ${BISON_constraint_parser_OUTPUTS})

target_link_libraries(unit_test
  ${PPL_PPL_LIBRARY}
  ${GMP_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_test(NAME unit_test
  COMMAND $<TARGET_FILE:unit_test>
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

if(BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

## It does not install the document by default
#    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
endif()

# INSTALL
install(TARGETS monaa DESTINATION bin)
