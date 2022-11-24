#include "Render.h"

Render::Render()
{
}

Render::Render(int width, int height)
{
    this->width = width;
    this->height = height;
    title = "N-Body";
}

Render::Render(int width, int height, char *title)
{
    this->width = width;
    this->height = height;
    this->title = title;
}

Render::~Render()
{
}

int Render::init()
{
    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    return 0;
}


void Render::render()
{
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
}

void Render::terminate()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}