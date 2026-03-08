cmake_minimum_required(VERSION 3.15...3.23)

project(CmakeConfigPackageTests 
  VERSION ${PROJECT_VERSION}
  LANGUAGES CXX)

# ---- Test as standalone project the exported config package ----

if(PROJECT_IS_TOP_LEVEL OR TEST_INSTALLED_VERSION)
  enable_testing()

  find_package(cpp_template_redux CONFIG REQUIRED) # for intro, project_options, ...

  if(NOT TARGET cpp_template_redux_options)
    message(FATAL_ERROR "Required config package not found!")
    return() # be strictly paranoid for Template Janitor github action! CK
  endif()
endif()

# ---- Dependencies ----

# GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Disable static analysis on GoogleTest targets
set_target_properties(gtest gtest_main gmock gmock_main
  PROPERTIES
    CXX_CLANG_TIDY ""
    C_CLANG_TIDY ""
    CXX_CPPCHECK ""
    C_CPPCHECK ""
)

include(GoogleTest)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "*test*.cpp")
add_executable(tests ${TEST_SOURCES})
target_link_libraries(
  tests
  PRIVATE cpp_template_redux::cpp_template_redux_warnings
          cpp_template_redux::cpp_template_redux_options
          GTest::gtest_main
          # Link against the app library (everything except main.cpp) so tests
          # have access to all our source code — websites, scraper, curl, etc.
          compiled_lib)

# Make the src/ directory visible to the test files so they can #include our headers.
target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

gtest_discover_tests(tests)

cpp_template_redux_configure_linker(tests)

if(WIN32 AND BUILD_SHARED_LIBS)
  add_custom_command(
    TARGET tests
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:tests> $<TARGET_FILE_DIR:tests>
    COMMAND_EXPAND_LISTS)
endif()
