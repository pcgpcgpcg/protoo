cmake_minimum_required(VERSION 3.21)
project(testlws)

#find_package(libwebsockets REQUIRED)

set(LIBWEBSOCKETS_DIR /usr/local/Cellar/libwebsockets/4.3.2/)

#Bring the headers, such as Student.h into the project
include_directories(${LIBWEBSOCKETS_DIR}/include)

#include_directories( ${LIBWEBSOCKETS_INCLUDE_DIRS} )

FILE(GLOB_RECURSE ALL_SOURCES
          RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
          ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${ALL_SOURCES})
#Can manually add the sources using the set command as follows:
#set(SOURCES src/mainapp.cpp src/Student.cpp)
#However, the file(GLOB...) or (GLOB_RECURSE...) allows for wildcard additions:
#file(GLOB SOURCES "src/*.cpp" main.cpp)

add_executable(testlws ${ALL_SOURCES})

target_link_libraries(testlws ${LIBWEBSOCKETS_DIR}/lib/libwebsockets.a)