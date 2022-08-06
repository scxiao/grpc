cmake_minimum_required(VERSION 3.5)
project (Test_Calculator)

set (CMAKE_CXX_STANDARD 14)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Threads)

# generate library for protobuf
set(PROTO_FILES calc.proto)
set(PROTO_LIBS calc)
add_library(${PROTO_LIBS} ${PROTO_FILES})
target_link_libraries(${PROTO_LIBS} PUBLIC protobuf::libprotobuf
                      gRPC::grpc
                      gRPC::grpc++)
target_include_directories(${PROTO_LIBS} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET ${PROTO_LIBS} LANGUAGE cpp)
protobuf_generate(TARGET ${PROTO_LIBS} LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

# set(PROTO_LIB addressbook)
# protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS addressbook.proto)
# add_library(${PROTO_LIB} STATIC ${PROTO_SRCS})
# include_directories(pb ${PROTOBUF_INCLUDE_DIR})

message("PROTOBUF_LIB = " ${PROTOBUF_LIBRARY})
message("cmake_current_src_dir = " ${CMAKE_SOURCE_DIR})
message("cmake_current_bin_dir = " ${CMAKE_BINARY_DIR})

set(test_files 
    calc_client.cc 
    calc_server.cc
    calc_async_client.cc
    calc_async_server.cc)

foreach(filepath_name ${test_files})
    get_filename_component(src_name ${filepath_name} NAME)
    get_filename_component(bin_name ${src_name} NAME_WE)
    message("source file: " ${src_name} " ---> bin: " ${bin_name})
    add_executable(${bin_name} cpp/${src_name})
    target_link_libraries(${bin_name} ${PROTOBUF_LIBRARY} gRPC::grpc++ gRPC::grpc++_reflection ${PROTO_LIBS})
endforeach(filepath_name)
