# Build Instructions

To build the project using CMake, follow these steps:

1. **Clone the repository:**
    ```sh
    git clone <repository-url>
    cd cpp_backend_tests
    ```

2. **Create and enter a build directory:**
    ```sh
    mkdir build
    cd build
    3. **Build the mongo-cxx-driver dependency:**

        The project depends on the [mongo-cxx-driver](https://www.mongodb.com/docs/languages/cpp/cpp-driver/current/get-started/). To build it from the `deps` directory:

        ```sh
        cd ../deps/mongo-cxx-driver-r3.10.1
        mkdir build
        cd build
        cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_CXX_STANDARD=17
        cmake --build .
        sudo cmake --build . --target install
        ```

4. **Install the Crow dependency:**

    **Option 2: Download a Crow release**
    Download and install the prebuilt Crow release for Linux:

    ```sh
    sudo apt install libasio-dev
    wget https://github.com/CrowCpp/Crow/releases/download/v1.2.0/Crow-1.2.0-Linux.deb
    sudo dpkg -i Crow-1.2.0-Linux.deb
    ```

5. **Install the Drogon dependency:**

    Follow the official [Drogon installation guide](https://github.com/drogonframework/drogon/wiki/ENG-02-Installation). For Ubuntu, you can use:

    ```sh
    sudo apt install libjsoncpp-dev uuid-dev zlib1g-dev 
    cd deps
    git clone https://github.com/drogonframework/drogon
    cd drogon
    git submodule update --init
    mkdir build
    cd build
    cmake ..
    make && sudo make install
    ```
3. **Configure the project with CMake:**
    ```sh
    cmake ..
    ```

4. **Build the project:**
    ```sh
    cmake --build .
    ```

5. **Run the tests (if available):**
    ```sh
    ctest
    ```