include(cmake/CPM.cmake)

# Done as a function so that updates to variables like
# CMAKE_CXX_FLAGS don't propagate out to other
# targets
function(cpp_template_redux_setup_dependencies)

  # For each dependency, see if it's
  # already been provided to us by a parent project

  if(NOT TARGET CURL::libcurl)
    find_package(CURL REQUIRED)
  endif()

  if(NOT TARGET RocksDB::rocksdb)
    find_package(RocksDB REQUIRED)
  endif()

endfunction()
