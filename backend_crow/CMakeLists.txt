set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/crow)

add_executable(crow_simple_test src/crow_simple.cpp)

find_package(Crow REQUIRED)
target_link_libraries(crow_simple_test PUBLIC Crow::Crow)

add_executable(crow_real_world_test src/crow_real_world.cpp)
target_link_libraries(crow_real_world_test PUBLIC Crow::Crow backend_common)