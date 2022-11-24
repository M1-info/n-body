#ifndef RENDER_H
#define RENDER_H

#include <GLFW/glfw3.h>

class Render
{
    private:
        GLFWwindow *window;
        int width;
        int height;
        char const *title;
    
    public:
        Render();
        Render(int width, int height);
        Render(int width, int height, char *title);
        ~Render();

        int init();
        void render();
        void terminate();

};

#endif // RENDER_H