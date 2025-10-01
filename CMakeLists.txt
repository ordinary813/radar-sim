cmake_minimum_required(VERSION 3.16)
project(RadarSim)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES "src/*.cpp")

add_executable(radar_sim ${SOURCES})