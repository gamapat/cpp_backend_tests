#####################################
# Define Project-Wide Settings
#####################################
cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

# Define the project name and language
project(CppBackendTests
	LANGUAGES CXX
	VERSION 1.1.1
)

# Set required C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set the output directory for the project
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(backend_common)
add_subdirectory(backend_crow)
add_subdirectory(backend_drogon)
add_subdirectory(backend_pistache)
add_subdirectory(backend_cpp_httplib)
add_subdirectory(backend_oatpp)
add_subdirectory(backend_boost)