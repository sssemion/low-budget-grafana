set(SRCS prometheus.h prometheus.cpp)

add_library(prometheus STATIC ${SRCS})

target_link_libraries(prometheus PRIVATE tsdb nlohmann_json::nlohmann_json)

if(TEST)
    add_subdirectory(tests)
endif()
