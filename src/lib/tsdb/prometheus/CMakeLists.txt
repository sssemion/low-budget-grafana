set(SRCS prometheus.h prometheus.cpp)

add_library(prometheus STATIC ${SRCS})

target_link_libraries(prometheus PRIVATE tsdb)

if(TEST)
    enable_testing()
    add_subdirectory(tests)
endif()
