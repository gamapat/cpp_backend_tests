set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/pistache)

add_executable(pistache_simple_test src/pistache_simple.cpp)
set(PISTACHE_LIB "/home/user/.pistache/lib/x86_64-linux-gnu/libpistache.so")
set(PISTACHE_INCLUDE "/home/user/.pistache/include/")
target_link_libraries(pistache_simple_test PUBLIC ${PISTACHE_LIB})
target_include_directories(pistache_simple_test PUBLIC ${PISTACHE_INCLUDE})

add_executable(pistache_real_world_test src/pistache_real_world.cpp)
target_link_libraries(pistache_real_world_test PUBLIC ${PISTACHE_LIB} backend_common)
target_include_directories(pistache_real_world_test PUBLIC ${PISTACHE_INCLUDE})