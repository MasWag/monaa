set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-deprecated-register")

project(MONAA CXX)

cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_FLAGS "-Wall -std=c++14")
# Hack to disable -march=native if it is not supported
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-flto -O3 -DRELEASE")
set(VERSION_MAJOR 0)
set(VERSION_MINOR 5)
set(VERSION_PATCH 2)

if(${CMAKE_VERSION} VERSION_LESS "3.12.0")
  add_definitions(
    "-DMONAA_VERSION=\"${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}\"")
else()
  add_compile_definitions(
    MONAA_VERSION="${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
endif()

# On macOS, search Homebrew for keg-only versions of Bison and Flex. Xcode does
# not provide new enough versions for us to use.
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(
        COMMAND brew --prefix bison
        RESULT_VARIABLE BREW_BISON
        OUTPUT_VARIABLE BREW_BISON_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(BREW_BISON EQUAL 0 AND EXISTS "${BREW_BISON_PREFIX}")
      message(STATUS
        "Found Bison keg installed by Homebrew at ${BREW_BISON_PREFIX}")
      set(CMAKE_FIND_ROOT_PATH "${BREW_BISON_PREFIX};${CMAKE_FIND_ROOT_PATH}")
    endif()

    execute_process(
        COMMAND brew --prefix flex
        RESULT_VARIABLE BREW_FLEX
        OUTPUT_VARIABLE BREW_FLEX_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(BREW_FLEX EQUAL 0 AND EXISTS "${BREW_FLEX_PREFIX}")
      message(STATUS
        "Found Flex keg installed by Homebrew at ${BREW_FLEX_PREFIX}")
        set(CMAKE_FIND_ROOT_PATH "${BREW_FLEX_PREFIX};${CMAKE_FIND_ROOT_PATH}")
    endif()
endif()

find_package(BISON 3.0 REQUIRED)
find_package(FLEX REQUIRED)
find_package(Boost REQUIRED COMPONENTS
  program_options unit_test_framework iostreams graph)
find_package(Eigen3 REQUIRED)

include_directories(
  monaa/
  libmonaa/
  ${BISON_INCLUDE_DIRS}
  ${FLEX_INCLUDE_DIRS}
  ${PROJECT_BINARY_DIR}
  ${Boost_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIR})

bison_target(TRE_PARSER
  monaa/tre_parser.yy
  ${CMAKE_CURRENT_BINARY_DIR}/tre_parser.tab.cc)
flex_target(TRE_LEXER
  monaa/tre_lexer.l
  ${CMAKE_CURRENT_BINARY_DIR}/tre_lexer.yy.cc)
add_flex_bison_dependency(TRE_LEXER TRE_PARSER)

## Config for Main monaa
add_executable(monaa
  monaa/main.cc
  libmonaa/intersection.cc
  libmonaa/ta2za.cc
  monaa/tre.cc
  monaa/intermediate_tre.cc
  ${FLEX_TRE_LEXER_OUTPUTS}
  ${BISON_TRE_PARSER_OUTPUTS})

target_link_libraries(monaa
#  profiler
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_GRAPH_LIBRARY})

target_include_directories(monaa
  PRIVATE
  .
  ${CMAKE_CURRENT_BINARY_DIR})
set_property(TARGET monaa
  PROPERTY CXX_STANDARD 14)

## Config for TRE2TA
add_executable(tre2ta EXCLUDE_FROM_ALL
  monaa/tre2ta.cc
  monaa/tre.cc
  monaa/intermediate_tre.cc
  libmonaa/intersection.cc
  ${FLEX_TRE_LEXER_OUTPUTS}
  ${BISON_TRE_PARSER_OUTPUTS})

target_link_libraries(tre2ta
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_GRAPH_LIBRARY})

target_include_directories(tre2ta
  PRIVATE
  .
  ${CMAKE_CURRENT_BINARY_DIR})
set_property(TARGET tre2ta
  PROPERTY CXX_STANDARD 14)

## Config for libmonaa
add_library(libmonaa STATIC EXCLUDE_FROM_ALL
  libmonaa/intersection.cc
  libmonaa/ta2za.cc
)

## We require rapidcheck for property-based testing
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/rapidcheck/CMakeLists.txt")
  ## Config for Test
  enable_testing()

  add_executable(unit_test EXCLUDE_FROM_ALL
    libmonaa/intersection.cc
    libmonaa/ta2za.cc
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
    test/interval_test.cc
    test/timed_automaton_parser_test.cc
    ${FLEX_TRE_LEXER_OUTPUTS}
    ${BISON_TRE_PARSER_OUTPUTS})

  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/rapidcheck/")

  target_include_directories(unit_test
    PRIVATE
    .
    ${CMAKE_CURRENT_BINARY_DIR}
    rapidcheck/extras/boost_test/include/)

  target_link_libraries(unit_test
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    rapidcheck)

  add_test(NAME unit_test
    COMMAND $<TARGET_FILE:unit_test>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
option(BUILD_DOCUMENTATION
  "Create and install the HTML based API documentation (requires Doxygen)"
  ${DOXYGEN_FOUND})

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
