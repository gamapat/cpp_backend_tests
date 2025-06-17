# define executable subdirectory as project output directory with subdirectory boost
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/boost)

add_executable(boost_simple_test src/boost_simple.cpp)

find_package(Boost CONFIG REQUIRED)
target_link_libraries(boost_simple_test PUBLIC Boost::Beast)

add_executable(boost_real_world_test src/boost_real_world.cpp)
target_link_libraries(boost_real_world_test PUBLIC Boost::Beast backend_common)