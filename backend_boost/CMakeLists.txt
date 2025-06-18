# define executable subdirectory as project output directory with subdirectory boost
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/boost)

add_executable(boost_simple_test src/boost_simple.cpp)

find_package(Boost COMPONENTS REQUIRED)
target_include_directories(boost_simple_test PRIVATE ${Boost_INCLUDE_DIRS})

add_executable(boost_real_world_test src/boost_real_world.cpp)
target_include_directories(boost_real_world_test PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(boost_real_world_test PUBLIC backend_common)