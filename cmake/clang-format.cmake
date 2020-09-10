# get all project files
file(GLOB_RECURSE ALL_SOURCE_FILES src/*.c src/*.cpp src/*.h src/*.hpp)

set(PROJECT_TRDPARTY_DIR ${PROJECT_SOURCE_DIR}/lib)

foreach (SOURCE_FILE ${ALL_SOURCE_FILES})
    string(FIND ${SOURCE_FILE} ${PROJECT_TRDPARTY_DIR} PROJECT_TRDPARTY_DIR_FOUND)
    if (NOT ${PROJECT_TRDPARTY_DIR_FOUND} EQUAL -1)
        list(REMOVE_ITEM ALL_SOURCE_FILES ${SOURCE_FILE})
    endif ()
endforeach ()

add_custom_target(
        clang_format
        COMMAND /usr/bin/clang-format-10
        -style=LLVM
        -i
        ${ALL_SOURCE_FILES}
)