find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(app app.cpp)

target_link_libraries(app
    PRIVATE
        OpenGL::GL
        glfw3
        imgui
        implot
        prometheus
        "-framework Cocoa"
        "-framework CoreFoundation"
        "-framework IOKit"
)
