set(APPS_HDRS)

set(APPS_SRCS
#    client.cpp
#    server.cpp
    protoc_gen_mstch.cpp
    my_app.cpp
    my_app/Consumer.cpp
    my_app/Producer.cpp
)

set(CMAKE_PREFIX_PATH /usr/local/lib)

add_library(apps SHARED STATIC ${APPS_SRCS} ${APPS_HDRS})

target_include_directories(apps PRIVATE
    $ENV{HOME}/local/include
    /usr/local/lib
)
target_link_libraries(apps PRIVATE
    fmt mstch protobuf common protoc stdc++fs
)

install(FILES ${APPS_HDRS} DESTINATION include/apps)
install(TARGETS apps DESTINATION lib)
