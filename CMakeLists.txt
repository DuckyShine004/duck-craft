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

pkg_check_modules(OPENAL REQUIRED openal)
pkg_check_modules(SNDFILE REQUIRED sndfile)

add_executable(duck-craft ${DUCK_CRAFT_SOURCES})

target_compile_definitions(duck-craft PRIVATE GLFW_INCLUDE_NONE)

target_include_directories(
  duck-craft PRIVATE ${OPENAL_INCLUDE_DIRS} ${SNDFILE_INCLUDE_DIRS}
                     ${CMAKE_SOURCE_DIR}/include)

find_library(OPENAL_FRAMEWORK OpenAL)

target_link_libraries(
  duck-craft
  PRIVATE ${OPENAL_LDFLAGS}
          ${SNDFILE_LDFLAGS}
          glfw
          fmt::fmt
          Boost::boost
          TBB::tbb
          FastNoise2::FastNoise)

target_compile_options(duck-craft PRIVATE $<$<CONFIG:Release>:-O3 -march=native
                                          -ffunction-sections -fdata-sections>)

target_link_options(duck-craft PRIVATE $<$<CONFIG:Release>:-Wl,--as-needed
                    -Wl,--gc-sections>)

set_property(TARGET duck-craft PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE
                                        TRUE)
target_compile_options(
  duck-craft PRIVATE $<$<CONFIG:Debug>:-O1 -g -fsanitize=address
                     -fno-omit-frame-pointer>)

target_link_options(duck-craft PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)

set_property(TARGET duck-craft PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG TRUE)

target_compile_options(
  duck-craft PRIVATE $<$<CONFIG:Profile>:-O2 -g -fno-omit-frame-pointer
                     -march=native>)

target_link_options(duck-craft PRIVATE $<$<CONFIG:Profile>:-Wl,--as-needed>)

set_property(TARGET duck-craft PROPERTY INTERPROCEDURAL_OPTIMIZATION_PROFILE
                                        FALSE)
