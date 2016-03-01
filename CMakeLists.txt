cmake_minimum_required(VERSION 3.4.3)

project(musicvisualizer)

cmake_policy(SET CMP0028 NEW)


# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)


# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)


file(GLOB_RECURSE source src/*.cpp)

include_directories(src)



# Find the Qt libraries
find_package(Qt5Core PATHS ${CMAKE_PREFIX_PATH})
find_package(Qt5Qml PATHS ${CMAKE_PREFIX_PATH})
find_package(Qt5Quick PATHS ${CMAKE_PREFIX_PATH})


set(RESOURCE ${CMAKE_SOURCE_DIR}/src/qml.qrc)
qt5_add_resources(RESOURCE_ADDED ${RESOURCE} ${RESOURCE_ADDED})

# Tell CMake to create the helloworld executable
add_executable(musicvisualizer ${source} ${RESOURCE_ADDED})

qt5_use_modules(musicvisualizer Core Qml Quick)