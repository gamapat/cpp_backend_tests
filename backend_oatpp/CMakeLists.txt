set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/oatpp)

add_executable(oatpp_simple_test src/oatpp_simple.cpp)

find_package(oatpp REQUIRED)
target_link_libraries(oatpp_simple_test PUBLIC oatpp::oatpp)

add_executable(oatpp_real_world_test src/oatpp_real_world.cpp)
target_link_libraries(oatpp_real_world_test PUBLIC oatpp::oatpp backend_common)