# define executable subdirectory as project output directory with subdirectory httplib
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/httplib)

add_executable(httplib_simple_test src/httplib_simple.cpp)

# Add include directory for cpp-httplib
target_include_directories(httplib_simple_test PUBLIC ${CMAKE_SOURCE_DIR}/deps/cpp-httplib ${CMAKE_SOURCE_DIR}/deps/json/include)

add_executable(httplib_real_world_test src/httplib_real_world.cpp)
target_link_libraries(httplib_real_world_test PUBLIC httplib::httplib backend_common)
target_include_directories(httplib_real_world_test PUBLIC ${CMAKE_SOURCE_DIR}/deps/cpp-httplib ${CMAKE_SOURCE_DIR}/deps/json/include)