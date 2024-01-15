cmake_minimum_required(VERSION 3.21)
project(protoo_demo)

#find_package(libwebsockets REQUIRED)
message(APPLE = ${APPLE})

set(CMAKE_CXX_STANDARD 17)

if(WIN32)
    message(STATUS "Current system is Windows")
endif()

if(APPLE)
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        message(STATUS "Current system is macOS")
        set(LIBWEBSOCKETS_DIR /usr/local/Cellar/libwebsockets/4.3.2)
        set(OPENSSL_DIR /usr/local/Cellar/openssl@1.1/1.1.1t)
        set(LIBUV_DIR /usr/local/Cellar/libuv/1.44.2)
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

FILE(GLOB_RECURSE all_source_files "client/src/*.cpp", "main.cpp")
FILE(GLOB_RECURSE all_head_files "client/include/*.h")

add_subdirectory(thirdparty/nlohmann_json)

add_executable(
            ${PROJECT_NAME} 
            ${all_head_files}
            ${all_source_files}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/thirdparty/nlohmann_json/single_include/nlohmann)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/client/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/client/include/transports)

target_include_directories(${PROJECT_NAME} PRIVATE ${LIBWEBSOCKETS_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBUV_DIR}/include)

target_link_libraries(${PROJECT_NAME} ${LIBWEBSOCKETS_DIR}/lib/libwebsockets.a 
                                      ${OPENSSL_DIR}/lib/libssl.a 
                                      ${OPENSSL_DIR}/lib/libcrypto.a
                                      ${LIBUV_DIR}/lib/libuv.a
                     )
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)