cmake_minimum_required(VERSION 3.4.3)

project(musicvisualizer)

cmake_policy(SET CMP0028 NEW)

add_definitions(-std=c++14)

set(CMAKE_BUILD_TYPE "Debug")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)


# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)


file(GLOB_RECURSE sources src/*.cpp)
file(GLOB_RECURSE headers src/*.h)

include_directories(src)


# Find the Qt libraries
find_package(Qt5Core PATHS ${CMAKE_PREFIX_PATH})
find_package(Qt5Qml PATHS ${CMAKE_PREFIX_PATH})
find_package(Qt5Quick PATHS ${CMAKE_PREFIX_PATH})
find_package(Qt5Multimedia PATHS ${CMAKE_PREFIX_PATH})


set(RESOURCE ${CMAKE_SOURCE_DIR}/src/qml.qrc)
qt5_add_resources(RESOURCE_ADDED ${RESOURCE} ${RESOURCE_ADDED})

add_executable(musicvisualizer ${sources} ${headers} ${RESOURCE_ADDED})

qt5_use_modules(musicvisualizer Core Qml Quick Multimedia)


# Find fftw3

find_path(FFTW3_INCLUDE_DIR
    NAMES
    fftw3.h
    PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include)

find_library(FFTW3_LIBRARY
    NAMES
    fftw3
    PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib)


set(FFTW3_INCLUDE_DIRS
    ${FFTW3_INCLUDE_DIR})

set(FFTW3_LIBRARIES
    ${FFTW3_LIBRARY})

if (FFTW3_INCLUDE_DIRS AND FFTW3_LIBRARIES)
    set(FFTW3_FOUND true)
endif (FFTW3_INCLUDE_DIRS AND FFTW3_LIBRARIES)

if (NOT FFTW3_FOUND)
    message(fatal_error "Could not find FFTW3!")
endif (NOT FFTW3_FOUND)

target_link_libraries(musicvisualizer ${FFTW3_LIBRARY})
