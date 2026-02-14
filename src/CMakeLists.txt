add_executable(compiled_app main.cpp)

target_link_libraries(
  compiled_app
  PRIVATE cpp_template_redux::cpp_template_redux_options
          cpp_template_redux::cpp_template_redux_warnings
          CURL::libcurl)

cpp_template_redux_configure_linker(compiled_app)
