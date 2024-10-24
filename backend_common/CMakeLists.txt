add_library(backend_common SHARED src/real_world.cpp)
target_include_directories(backend_common PUBLIC include)

# Find and link MongoDB C++ driver
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)

# Add mongocxx and bsoncxx include directories
set(MONGOCXX_INCLUDE_DIRS "/usr/local/include/mongocxx/v_noabi")
set(BSONCXX_INCLUDE_DIRS "/usr/local/include/bsoncxx/v_noabi" "/usr/local/include/bsoncxx/v_noabi/bsoncxx/third_party/mnmlstc/")
message(STATUS "MONGOCXX_INCLUDE_DIRS: ${MONGOCXX_INCLUDE_DIRS}")
message(STATUS "BSONCXX_INCLUDE_DIRS: ${BSONCXX_INCLUDE_DIRS}")
target_include_directories(backend_common PRIVATE ${MONGOCXX_INCLUDE_DIRS} ${BSONCXX_INCLUDE_DIRS})

set(MONGOCXX_LIBRARIES "/usr/local/lib/libmongocxx.so")
set(BSONCXX_LIBRARIES "/usr/local/lib/libbsoncxx.so")
target_link_libraries(backend_common PRIVATE ${MONGOCXX_LIBRARIES} ${BSONCXX_LIBRARIES})

# export the target
install(TARGETS backend_common
    EXPORT CppBackendTests
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)