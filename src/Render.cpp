#include "Render.h"
#include <iostream>

Render::Render()
{
    m_width = 800;
    m_height = 600;
    m_title = "N-Body";
    m_clear_color = {1.0f, 0.0f, 0.0f};
}

Render::Render(int width, int height)
{
    m_width = width;
    m_height = height;
    m_title = "N-Body";
    m_clear_color = {1.0f, 0.0f, 0.0f};
}

Render::Render(int width, int height, char *title)
{
    m_width = width;
    m_height = height;
    m_title = title;
    m_clear_color = {1.0f, 0.0f, 0.0f};
}

Render::~Render()
{
    shutdown();
    m_window = nullptr;
}

void Render::init()
{

    if (!glfwInit())
        return;

    /* Create a windowed mode window and its OpenGL context */
    m_window = glfwCreateWindow(m_width, m_height, m_title, NULL, NULL);
    if (!m_window)
    {
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_window);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetWindowSizeCallback(m_window, resize);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);
}

void Render::render()
{

    ImGuiIO &io = ImGui::GetIO();

    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {

            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

            const ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

            if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
                window_flags |= ImGuiWindowFlags_NoBackground;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("N-Body", nullptr, window_flags);

            ImGui::PopStyleVar();

            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID("N-Body");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            }

            ImGui::Begin("Background Color");
            ImGui::Text("Change the background color");
            ImGui::ColorEdit3("Clear Color", (float *)&m_clear_color);
            ImGui::End();

            ImGui::Begin("Frame Rate");
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();

            ImGui::End();
        }

        /* Render here */
        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(m_clear_color[0], m_clear_color[1], m_clear_color[2], 1.0f);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        glfwSwapBuffers(m_window);
    }
}

void Render::shutdown()
{

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Render::resize(GLFWwindow *window, int width, int height)
{
    Render *render = reinterpret_cast<Render *>(glfwGetWindowUserPointer(window));

    if (width == render->m_width && height == render->m_height)
        return;

    render->m_width = width;
    render->m_height = height;

    glfwSetWindowSize(render->m_window, render->m_width, render->m_height);
    glViewport(0, 0, render->m_width, render->m_height);
}