cmake_minimum_required(VERSION 3.5)
project (Test_ProtoBuf)

set (CMAKE_CXX_STANDARD 14)

find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# find_package(PkgConfig REQUIRED)
# pkg_search_module(GRPC REQUIRED grpc)
# pkg_search_module(GRPCPP REQUIRED grpc++>=1.22.0)

message("PROTOBUF_LIB = " ${PROTOBUF_LIBRARY})
include_directories(pb ${PROTOBUF_INCLUDE_DIR})

message("cmake_current_src_dir = " ${CMAKE_SOURCE_DIR})
message("cmake_current_bin_dir = " ${CMAKE_BINARY_DIR})

set(test_files write_message.cpp read_message.cpp person_client.cpp person_server.cpp)
#add_library(reduce SHARED ${reduce_SRC})

#set(test_files test_reduce.cpp)

foreach(filepath_name ${test_files})
    get_filename_component(src_name ${filepath_name} NAME)
    get_filename_component(bin_name ${src_name} NAME_WE)
    message("source file: " ${src_name} " ---> bin: " ${bin_name})
    add_executable(${bin_name} cpp/${src_name} pb/addressbook.pb.cc pb/addressbook.grpc.pb.cc)
    target_link_libraries(${bin_name} ${PROTOBUF_LIBRARY} gRPC::grpc++ gRPC::grpc++_reflection)
endforeach(filepath_name)

