include(FetchContent)

if (WIN32)
    # Отключаем проверку TLS сертификатов на Windows
    set(CMAKE_TLS_VERIFY OFF CACHE BOOL "Disable TLS certificate verification" FORCE)
endif()

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

FetchContent_Declare(curl_src
    URL https://curl.se/download/curl-8.5.0.tar.gz
)
FetchContent_GetProperties(curl_src)
if (NOT curl_src_POPULATED)
  FetchContent_Populate(curl_src)
endif()
set(HTTP_ONLY ON CACHE BOOL "Disable other protocols" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libs" FORCE)
set(BUILD_CURL_EXE OFF CACHE BOOL "Do not build curl exe" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Disable cURL tests" FORCE)
add_subdirectory(${curl_src_SOURCE_DIR} ${curl_src_BINARY_DIR})

if(TEST)
    include(ExternalProject)
    find_package(Git REQUIRED)

    ExternalProject_Add(
        doctest
        PREFIX ${CMAKE_BINARY_DIR}/doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        TIMEOUT 10
        UPDATE_COMMAND ${GIT_EXECUTABLE} pull
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
    )

    ExternalProject_Get_Property(doctest source_dir)
    set(DOCTEST_INCLUDE_DIR ${source_dir}/doctest CACHE INTERNAL "Path to include folder for doctest")
    enable_testing()
endif()
