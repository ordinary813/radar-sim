cmake_minimum_required(VERSION 3.16)
project(RadarSim)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES "src/*.cpp")
add_executable(radar_sim ${SOURCES})

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
target_link_libraries(radar_sim sfml-graphics sfml-window sfml-system)