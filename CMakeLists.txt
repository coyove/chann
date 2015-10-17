cmake_minimum_required(VERSION 3.3)
project(cchan)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    lib/mongoose/mongoose.c
    lib/mongoose/mongoose.h
    lib/unqlite/unqlite.c
    lib/unqlite/unqlite.h
    src/cookie.cpp
    src/cookie.h
    src/general.cpp
    src/general.h
    src/helper.cpp
    src/helper.h
    src/lang.h
    src/main.cpp
    src/templates.h)

add_executable(cchan ${SOURCE_FILES})
target_link_libraries(cchan wsock32 ws2_32)