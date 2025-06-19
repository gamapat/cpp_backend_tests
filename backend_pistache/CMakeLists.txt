set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/pistache)

find_package(PkgConfig)
pkg_check_modules(Pistache REQUIRED IMPORTED_TARGET libpistache)

add_executable(pistache_simple_test src/pistache_simple.cpp)
target_link_libraries(pistache_simple_test PUBLIC PkgConfig::Pistache)

add_executable(pistache_real_world_test src/pistache_real_world.cpp)
target_link_libraries(pistache_real_world_test PUBLIC PkgConfig::Pistache backend_common)