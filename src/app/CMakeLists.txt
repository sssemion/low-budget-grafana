find_package(OpenGL REQUIRED)

add_executable(app app.cpp)

target_link_libraries(app
    PRIVATE
        OpenGL::GL
        glfw
        imgui
        implot
        prometheus
)
