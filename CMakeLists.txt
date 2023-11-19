cmake_minimum_required(VERSION 3.21)
project(protoo_demo)

#find_package(libwebsockets REQUIRED)

if (WIN32)
    # Windows-specific commands here
elseif (UNIX)
    # Unix-specific commands here
elseif (APPLE)
    # Apple-specific commands here
    set(LIBWEBSOCKETS_DIR /usr/local/Cellar/libwebsockets/4.3.2)
    set(OPENSSL_DIR /usr/local/Cellar/openssl@1.1/1.1.1q)
    set(LIBUV_DIR /usr/local/Cellar/libuv/1.44.2)
elseif (ANDROID)
    # Android-specific commands here
elseif (IOS)
    # iOS-specific commands here
endif()

#Bring the headers, such as Student.h into the project
#include_directories(${LIBWEBSOCKETS_DIR}/include)

FILE(GLOB_RECURSE ALL_SOURCES
          RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
          ${CMAKE_CURRENT_SOURCE_DIR}/client/src/*.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/client/include/*.h)
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${ALL_SOURCES})
#Can manually add the sources using the set command as follows:
#set(SOURCES src/mainapp.cpp src/Student.cpp)
#However, the file(GLOB...) or (GLOB_RECURSE...) allows for wildcard additions:
#file(GLOB SOURCES "src/*.cpp" main.cpp)

add_executable(protoo_demo ${ALL_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${LIBWEBSOCKETS_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBUV_DIR}/include)

target_link_libraries(${PROJECT_NAME} ${LIBWEBSOCKETS_DIR}/lib/libwebsockets.a 
                                      ${OPENSSL_DIR}/lib/libssl.a 
                                      ${OPENSSL_DIR}/lib/libcrypto.a
                                      ${LIBUV_DIR}/lib/libuv.a
                     )