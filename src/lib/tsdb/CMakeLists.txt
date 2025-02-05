set(SRCS tsdb.cpp)

add_library(tsdb STATIC ${SRCS})

target_link_libraries(tsdb PRIVATE CURL::libcurl)

add_subdirectory(prometheus)
