
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_CLANG_TIDY clang-tidy-10 -checks=-*,readability-*,cert-*,modernize-*)

set(CMAKE_CXX_CLANG_TIDY clang-tidy-10)

add_compile_options("-std=c++1z")
add_compile_options("-Og")
add_compile_options("-g")

add_compile_options("-Wall")
add_compile_options("-Wextra")
add_compile_options("-pedantic")


#add_compile_options("-Werror")
add_compile_options("-pedantic-errors")

add_compile_options("-fsanitize=address,undefined")
add_compile_options("-fno-omit-frame-pointer")

add_link_options("-fsanitize=address,undefined")



