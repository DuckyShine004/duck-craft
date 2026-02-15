cmake_minimum_required(VERSION 3.10)
project(
  duck-craft
  VERSION 0.0.1
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

file(GLOB_RECURSE DUCK_CRAFT_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp"
     "${CMAKE_SOURCE_DIR}/src/*.c")

find_package(PkgConfig REQUIRED)
find_package(fmt REQUIRED)
find_package(Boost REQUIRED)
find_package(TBB REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(FastNoise2 CONFIG REQUIRED)

add_executable(duck-craft ${DUCK_CRAFT_SOURCES})

target_compile_definitions(duck-craft PRIVATE GLFW_INCLUDE_NONE)

target_include_directories(duck-craft PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(duck-craft PRIVATE glfw fmt::fmt Boost::boost TBB::tbb
                                         FastNoise2::FastNoise)

target_compile_options(duck-craft PRIVATE $<$<CONFIG:Release>:-O3 -march=native
                                          -ffunction-sections -fdata-sections>)

target_link_options(duck-craft PRIVATE $<$<CONFIG:Release>:-Wl,--as-needed
                    -Wl,--gc-sections>)

set_property(TARGET duck-craft PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE
                                        TRUE)
