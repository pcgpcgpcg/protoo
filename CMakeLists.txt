cmake_minimum_required(VERSION 3.21)
project(protoo_demo)

#find_package(libwebsockets REQUIRED)
message(APPLE = ${APPLE})

set(CMAKE_CXX_STANDARD 17)

if(WIN32)
    message(STATUS "Current system is Windows")
    set(LIBWEBSOCKETS_DIR ${CMAKE_CURRENT_LIST_DIR}/thirdparty/libwebsockets)
    set(OPENSSL_DIR ${CMAKE_CURRENT_LIST_DIR}/thirdparty/OpenSSL-Win64)
    # set(LIBUV_DIR /usr/local/Cellar/libuv/1.47.0)
endif()

if(APPLE)
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        message(STATUS "Current system is macOS")
        set(LIBWEBSOCKETS_DIR /usr/local/Cellar/libwebsockets/4.3.3)
        set(OPENSSL_DIR /usr/local/Cellar/openssl@3/3.2.0_1)
        set(LIBUV_DIR /usr/local/Cellar/libuv/1.47.0)
    endif()
endif()

if(UNIX AND NOT APPLE)
    message(STATUS "Current system is Linux")
endif()

if(ANDROID)
    message(STATUS "Current system is Android")
endif()

if(IOS)
    message(STATUS "Current system is iOS")
endif()



# FILE(GLOB_RECURSE ALL_SOURCES
#          RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
#         ${CMAKE_CURRENT_SOURCE_DIR}/client/src/*.cpp
#          ${CMAKE_CURRENT_SOURCE_DIR}/client/include/*.h)
# source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${ALL_SOURCES})
#Can manually add the sources using the set command as follows:
#set(SOURCES src/mainapp.cpp src/Student.cpp)
#However, the file(GLOB...) or (GLOB_RECURSE...) allows for wildcard additions:
#file(GLOB SOURCES "src/*.cpp" main.cpp)

message(STATUS PROJECT_NAME = ${PROJECT_NAME})
 message(STATUS CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR})
 message(STATUS CMAKE_CURRENT_LIST_DIR = ${CMAKE_CURRENT_LIST_DIR})

FILE(GLOB_RECURSE all_source_files "client/src/*.cpp", "main.cpp", "qrtm.cpp")
FILE(GLOB_RECURSE all_head_files "client/include/*.h", "qrtm.h")

add_executable(
            ${PROJECT_NAME} 
            ${all_head_files}
            ${all_source_files}
)

message(STATUS "Include path: ${CMAKE_CURRENT_LIST_DIR}/client/include/transports")

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/thirdparty/nlohmann)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/client/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/client/include/transports)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/client/include/utils)

target_include_directories(${PROJECT_NAME} PRIVATE ${LIBWEBSOCKETS_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBUV_DIR}/include)

if(APPLE)
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        message(STATUS "Current system is macOS")
target_link_libraries(${PROJECT_NAME} ${LIBWEBSOCKETS_DIR}/lib/libwebsockets.a 
                                      ${OPENSSL_DIR}/lib/libssl.a 
                                      ${OPENSSL_DIR}/lib/libcrypto.a
                                      ${LIBUV_DIR}/lib/libuv.a
                     )
    endif()
endif()

if(WIN32)
    message(STATUS "Current system is Windows")
    target_link_libraries(${PROJECT_NAME} ${LIBWEBSOCKETS_DIR}/lib/debug/websockets.lib 
    ${OPENSSL_DIR}/lib/libssl.lib 
    ${OPENSSL_DIR}/lib/libcrypto.lib
    # ${LIBUV_DIR}/lib/libuv.a
)
endif()