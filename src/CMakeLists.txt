# Collect all source files except main.cpp into a library so that both the
# app and the test binary can link against the same compiled code. If we
# included main.cpp here, the test binary would end up with two main()
# definitions (one from our code, one from GTest) and the linker would error.
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS "*.cpp")
list(FILTER LIB_SOURCES EXCLUDE REGEX "main\\.cpp$")
add_library(compiled_lib ${LIB_SOURCES})

target_link_libraries(
  compiled_lib
  PUBLIC cpp_template_redux::cpp_template_redux_options
         cpp_template_redux::cpp_template_redux_warnings
         CURL::libcurl
         RocksDB::rocksdb)

# The runnable app is just main.cpp wired to the library above.
add_executable(compiled_app main.cpp)

target_link_libraries(
  compiled_app
  PRIVATE compiled_lib)

cpp_template_redux_configure_linker(compiled_app)
