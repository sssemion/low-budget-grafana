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

find_package(OpenGL REQUIRED)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
FetchContent_GetProperties(glfw)
if(NOT glfw_POPULATED)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "Disable GLFW documentation build" FORCE)
    FetchContent_Populate(glfw)
    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
endif()

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.8
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp # todo remove
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC
    OpenGL::GL
    glfw
)

FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG v0.16
)
FetchContent_MakeAvailable(implot)

add_library(implot STATIC
    ${implot_SOURCE_DIR}/implot_demo.cpp  # todo remove
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
    ${implot_SOURCE_DIR}/implot.h
    ${implot_SOURCE_DIR}/implot_internal.h
)
target_include_directories(implot PUBLIC ${implot_SOURCE_DIR})
target_link_libraries(implot PUBLIC imgui)
