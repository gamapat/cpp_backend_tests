# Setting Up a CMake Project

This guide will help you set up, build, and run a CMake project for your C++ backend using Crow.

## Prerequisites

- CMake (version 3.10 or higher)
- A C++ compiler (e.g., g++, clang++)
- Build Crow from deps/Crow
- Build mongodb driver from deps/mongo. Instruction https://www.mongodb.com/docs/languages/cpp/cpp-driver/current/installation/linux/

## Project Structure

```
/home/user/article/cpp_backend/backend_crow/
├── CMakeLists.txt
├── src/
│   └── main.cpp
└── build/
```

## CMakeLists.txt

Create a `CMakeLists.txt` file in the root directory with the following content:

```cmake
cmake_minimum_required(VERSION 3.10)
project(backend_crow)

set(CMAKE_CXX_STANDARD 17)

# Add the executable
add_executable(backend_crow src/main.cpp)

# Include Crow (assuming Crow is installed or available in your include path)
include_directories(/path/to/crow/include)
```

## src/main.cpp

Create a `main.cpp` file in the `src` directory with a simple Crow application:

```cpp
#include "crow.h"

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        return "Hello, World!";
    });

    app.port(18080).multithreaded().run();
}
```

## Building the Project

1. Navigate to the project directory:
    ```sh
    cd /home/user/article/cpp_backend/backend_crow/
    ```

2. Create a `build` directory and navigate into it:
    ```sh
    mkdir build && cd build
    ```

3. Run CMake to generate the build files:
    ```sh
    cmake ..
    ```

4. Build the project:
    ```sh
    make
    ```

## Running the Project

After building the project, you can run the executable:

```sh
./backend_crow
```

Your Crow application should now be running on `http://localhost:18080`.