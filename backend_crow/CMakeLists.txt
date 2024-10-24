#####################################
# Define Project-Wide Settings
#####################################
cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

# Define the project name and language
project(CrowTestApp
	LANGUAGES CXX
	VERSION 1.1.1
)

# Set required C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

#####################################
# Define Targets
#####################################

add_executable(crow_simple_test src/crow_simple.cpp)

find_package(Crow REQUIRED)
target_link_libraries(crow_simple_test PUBLIC Crow::Crow)

# Find and link MongoDB C++ driver
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)

# set BSONCXX_POLY_USE_IMPLS=ON for crow_real_world_test
add_definitions(-DBSONCXX_POLY_USE_IMPLS=ON)
add_executable(crow_real_world_test src/crow_real_world.cpp)

# Add mongocxx and bsoncxx include directories
set(MONGOCXX_INCLUDE_DIRS "/usr/local/include/mongocxx/v_noabi")
set(BSONCXX_INCLUDE_DIRS "/usr/local/include/bsoncxx/v_noabi" "/usr/local/include/bsoncxx/v_noabi/bsoncxx/third_party/mnmlstc/")
message(STATUS "MONGOCXX_INCLUDE_DIRS: ${MONGOCXX_INCLUDE_DIRS}")
message(STATUS "BSONCXX_INCLUDE_DIRS: ${BSONCXX_INCLUDE_DIRS}")
target_include_directories(crow_real_world_test PUBLIC ${MONGOCXX_INCLUDE_DIRS} ${BSONCXX_INCLUDE_DIRS})

set(MONGOCXX_LIBRARIES "/usr/local/lib/libmongocxx.so")
set(BSONCXX_LIBRARIES "/usr/local/lib/libbsoncxx.so")
target_link_libraries(crow_real_world_test PUBLIC Crow::Crow ${MONGOCXX_LIBRARIES} ${BSONCXX_LIBRARIES})