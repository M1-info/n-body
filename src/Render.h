#ifndef RENDER_H
#define RENDER_H

#define GLSL_VERSION "#version 410"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

class Render
{
private:
    GLFWwindow *m_window;
    int m_width;
    int m_height;
    char const *m_title;
    glm::vec3 m_clear_color;

public:
    Render();
    Render(int width, int height);
    Render(int width, int height, char *title);
    ~Render();

    void init();
    void render();
    void shutdown();
    static void resize(GLFWwindow *window, int width, int height);
};

#endif // RENDER_H