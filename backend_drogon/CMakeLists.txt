# define executable subdirectory as project output directory with subdirectory drogon
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/drogon)

add_executable(drogon_simple_test src/drogon_simple.cpp)

find_package(Drogon CONFIG REQUIRED)
target_link_libraries(drogon_simple_test PUBLIC Drogon::Drogon)

add_executable(drogon_real_world_test src/drogon_real_world.cpp)
target_link_libraries(drogon_real_world_test PUBLIC Drogon::Drogon backend_common)