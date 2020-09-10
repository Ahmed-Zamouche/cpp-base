cmake_minimum_required(VERSION 3.17.0)

project(Acme VERSION 0.1.0)

#export CC=/usr/bin/clang-10
#export CXX=/usr/bin/clang++-10

set(CMAKE_C_COMPILER "/usr/bin/clang-10")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++-10")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Google Test Framwork
option(ENABLE_TESTS "Enable build testing" ON)
if(${ENABLE_TESTS})
    include(CTest) # this module will automatically enable_testing()
    enable_testing()
    add_subdirectory(thirdparty/googletest)
endif()

include(cmake/compile_options.cmake)

# External library
add_subdirectory(thirdparty/fmt)

add_subdirectory(datamodel)
add_subdirectory(protocols)
add_subdirectory(src)
add_subdirectory(mains)


include(cmake/clang-format.cmake)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)

