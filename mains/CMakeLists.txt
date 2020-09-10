add_executable(protoc_gen_mstch main_protoc_gen_mstch.cpp)

target_link_libraries(protoc_gen_mstch apps)

install(TARGETS protoc_gen_mstch DESTINATION bin)



add_executable(my_app main_my_app.cpp)

target_link_libraries(my_app apps)

install(TARGETS my_app DESTINATION bin)
