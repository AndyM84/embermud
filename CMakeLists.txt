cmake_minimum_required(VERSION 3.10)
project(embermud VERSION 1.0 LANGUAGES C)

add_executable(ember
    src/act_basic.c
    src/ban.c
    src/bit.c
    src/comm.c
    src/const.c
    src/db.c
    src/handler.c
    src/interp.c
    src/mem.c
    src/random.c
    src/recycle.c
    src/save.c
    src/ssm.c
    src/string.c
    src/update.c
)

target_include_directories(ember PRIVATE src)
target_compile_options(ember PRIVATE -Wall -ggdb -O3)

if(WIN32)
    target_compile_definitions(ember PRIVATE WIN32)
    target_link_libraries(ember PRIVATE ws2_32)
else()
    target_compile_definitions(ember PRIVATE LINUX)
    target_link_libraries(ember PRIVATE m)
endif()
