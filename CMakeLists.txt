cmake_minimum_required ( VERSION 3.1 )
set (PROJECT_NAME CppClient2)

project(${PROJECT_NAME})

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_GENERATOR_PLATFORM x64)

find_package(Protobuf REQUIRED)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.66.0 COMPONENTS thread REQUIRED)

set(HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cppclient)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/protos)
set(HEADERS
	${HEADER_DIR}/Client.h
    ${HEADER_DIR}/HeaderManager.h
    ${HEADER_DIR}/IPacket.h
    ${HEADER_DIR}/Macros.h
    ${HEADER_DIR}/OPacket.h
    ${HEADER_DIR}/PacketManager.h
    ${HEADER_DIR}/PKey.h
    ${HEADER_DIR}/PKeyOwner.h
    ${HEADER_DIR}/TCPConnection.h
    ${HEADER_DIR}/UDPConnection.h)

set(SOURCES
	${SOURCE_DIR}/Client.cpp
    ${SOURCE_DIR}/HeaderManager.cpp
    ${SOURCE_DIR}/IPacket.cpp
    ${SOURCE_DIR}/OPacket.cpp
    ${SOURCE_DIR}/PacketManager.cpp
    ${SOURCE_DIR}/PKey.cpp
    ${SOURCE_DIR}/PKeyOwner.cpp
    ${SOURCE_DIR}/TCPConnection.cpp
    ${SOURCE_DIR}/UDPConnection.cpp)

set (PROTOS 
    ${PROTO_DIR}/PackFW.proto)

include_directories(
    ${HEADER_DIR})

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS})

#Include protobuf pb.cc and pb.h in binary
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#Generate pb.cc and pb.h from proto files
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTOS})

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES} ${PROTO_HDRS} ${PROTO_SRCS})

target_link_libraries(${PROJECT_NAME} 
    ${Boost_LIBRARIES}
    ${Protobuf_LIBRARIES})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED 11)