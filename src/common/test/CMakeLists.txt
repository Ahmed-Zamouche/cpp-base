set(COMMON_HDRS

)

set(COMMON_SRCS
    TestCommon.cpp
)

add_library(test_common SHARED STATIC ${COMMON_SRCS} ${COMMON_HDRS})

add_executable(test_common_run /dev/null)

add_test(NAME test_common COMMAND test_common_run)

target_link_libraries(test_common_run PRIVATE
    -Wl,--whole-archive
    test_common
    common
    -Wl,--no-whole-archive
    gmock
    gtest_main
)

