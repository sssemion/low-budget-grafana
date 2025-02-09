find_package(OpenGL REQUIRED)

add_executable(app app.cpp constants.h utils.h utils.cpp)

target_link_libraries(app
    PRIVATE
        OpenGL::GL
        glfw
        imgui
        implot
        prometheus
)
