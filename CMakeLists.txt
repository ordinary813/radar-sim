cmake_minimum_required(VERSION 3.16)
project(RadarSim)

set(CMAKE_CXX_STANDARD 17)

add_executable(radar_sim
    src/main.cpp
    src/Radar.cpp
    src/Body.cpp
)
