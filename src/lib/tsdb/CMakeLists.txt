set(SRCS tsdb.cpp)

add_library(tsdb STATIC ${SRCS})

find_package(CURL REQUIRED)
target_link_libraries(tsdb PRIVATE CURL::libcurl)

add_subdirectory(prometheus)
