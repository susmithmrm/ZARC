cmake_minimum_required(VERSION 3.21)
project(zarc_someip_fixture LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(vsomeip3 REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Threads REQUIRED)
set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../interfaces")
set(GENERATED "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GENERATED}")
add_custom_command(
  OUTPUT "${GENERATED}/zarc_service.pb.cc" "${GENERATED}/zarc_service.pb.h"
         "${GENERATED}/zarc_service.grpc.pb.cc" "${GENERATED}/zarc_service.grpc.pb.h"
  COMMAND protobuf::protoc
  ARGS --proto_path=${PROTO_DIR} --cpp_out=${GENERATED}
       --grpc_out=${GENERATED} --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
       "${PROTO_DIR}/zarc_service.proto"
  DEPENDS "${PROTO_DIR}/zarc_service.proto" protobuf::protoc gRPC::grpc_cpp_plugin
  VERBATIM)
add_library(zarc_semantic_rpc STATIC "${GENERATED}/zarc_service.pb.cc" "${GENERATED}/zarc_service.grpc.pb.cc")
target_include_directories(zarc_semantic_rpc PUBLIC "${GENERATED}")
target_link_libraries(zarc_semantic_rpc PUBLIC protobuf::libprotobuf gRPC::grpc++)
add_executable(zarc_someip_service service.cpp)
target_link_libraries(zarc_someip_service PRIVATE vsomeip3 zarc_semantic_rpc Threads::Threads)
add_executable(zarc_someip_peer peer.cpp)
target_link_libraries(zarc_someip_peer PRIVATE vsomeip3 Threads::Threads)
