cmake_minimum_required(VERSION 3.25)
project(2_5d_Lands)

set(CMAKE_CXX_STANDARD 20)

# Find the SDL2 package installed via Homebrew
find_package(SDL2 REQUIRED)

# Add executable
add_executable(2_5d_Lands main.cpp Noise.cpp World.cpp Renderer.cpp Player.cpp)

# Add SDL2 include directories
target_include_directories(2_5d_Lands PRIVATE ${SDL2_INCLUDE_DIRS})

# Link SDL2 libraries
target_link_libraries(2_5d_Lands PRIVATE ${SDL2_LIBRARIES})