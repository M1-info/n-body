#ifndef RENDER_H
#define RENDER_H

#define GLSL_VERSION "#version 410"

#include "glad/include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "utils.h"

class Render
{
private:
    GLFWwindow *m_window;
    int m_width;
    int m_height;
    char const *m_title;
    glm::vec3 m_clear_color;

    GLuint m_vao;
    GLuint m_points_vbo;
    GLuint m_masses_vbo;
    GLuint m_shader;

public:
    Render();
    Render(int width, int height);
    Render(int width, int height, char *title);
    ~Render();

    void init(int vbo_size, double *masses);
    void draw(double *vbo, int vbo_size);
    void shutdown();
    void createShader();
    void setUpBuffers(int vbo_size, double *masses);
};

#endif // RENDER_H