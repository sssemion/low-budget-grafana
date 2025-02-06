add_executable(test_prometheus test_prometheus.cpp)

target_include_directories(test_prometheus PUBLIC ${DOCTEST_INCLUDE_DIR})

target_link_libraries(test_prometheus PRIVATE prometheus stdc++ m)

add_test(NAME test_prometheus COMMAND test_prometheus)
