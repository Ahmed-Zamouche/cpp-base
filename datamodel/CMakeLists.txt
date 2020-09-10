find_package(Protobuf REQUIRED)

set(Protobuf_USE_STATIC_LIBS ON)

set(PROTO_SRCS
  ${CMAKE_SOURCE_DIR}/protocols/request.proto
  ${CMAKE_SOURCE_DIR}/protocols/response.proto
  ${CMAKE_SOURCE_DIR}/protocols/service.proto
)

set(DATA_MODEL_HDRS
  Request.hpp
  Response.hpp
  Service.hpp
)

set(DATA_MODEL_SRCS
  Request.cpp
  Response.cpp
  Service.cpp
)


add_custom_command(OUTPUT ${DATA_MODEL_HDRS} ${DATA_MODEL_SRCS}
  COMMAND protoc --plugin=protoc-gen-mstch=../mains/protoc_gen_mstch -I${Protobuf_INCLUDE_DIRS} -I${CMAKE_SOURCE_DIR}/protocols --mstch_out=. ${PROTO_SRCS}
#  WORKING_DIRECTORY
  DEPENDS ${PROTO_SRCS}
  DEPENDS protoc_gen_mstch
  DEPENDS ${CMAKE_SOURCE_DIR}/datamodel/tmplt

  COMMENT "Compiling data models"
  VERBATIM
)

file(COPY ${CMAKE_SOURCE_DIR}/datamodel/tmplt DESTINATION  ../datamodel)

add_custom_target(datamodel DEPENDS ${DATA_MODEL_HDRS} ${DATA_MODEL_SRCS})

#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
#    request.proto
#    response.proto
#    service.proto)

#add_library(datamodel STATIC ${PROTO_SRCS} ${PROTO_HDRS})

#install(FILES ${PROTO_HDRS} DESTINATION include/datamodel)
#install(TARGETS datamodel DESTINATION lib)


