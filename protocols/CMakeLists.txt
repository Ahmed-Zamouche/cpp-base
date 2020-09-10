find_package(Protobuf REQUIRED)

set(Protobuf_USE_STATIC_LIBS ON)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
    request.proto
    response.proto
    service.proto)

add_library(protocols STATIC ${PROTO_SRCS} ${PROTO_HDRS})

install(FILES ${PROTO_HDRS} DESTINATION include/protocols)
install(TARGETS protocols DESTINATION lib)

