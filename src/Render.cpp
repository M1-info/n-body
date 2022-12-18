#include "Render.h"

Render::Render()
{
    m_width = 1000;
    m_height = 800;
    m_title = "N-Body";
    m_clear_color = {0.0f, 0.0f, 0.0f};
}

Render::Render(int width, int height)
{
    m_width = width;
    m_height = height;
    m_title = "N-Body";
    m_clear_color = {0.0f, 0.0f, 0.0f};
}

Render::Render(int width, int height, char *title)
{
    m_width = width;
    m_height = height;
    m_title = title;
    m_clear_color = {0.0f, 0.0f, 0.0f};
}

Render::~Render()
{
}

void Render::init(int vbo_size, double *masses)
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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    glEnable(GL_PROGRAM_POINT_SIZE);

    createShader();
    setUpBuffers(vbo_size, masses);

    glm::mat4 projection = glm::ortho(-500.0f, 500.0f, -500.0f, 500.0f, -1.0f, 1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = projection * view * model;

    glUseProgram(m_shader);
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "mvp"), 1, GL_FALSE, &mvp[0][0]);
    glUseProgram(0);
}

void Render::draw(double *vbo, int vbo_size)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, m_points_vbo);
    glBufferData(GL_ARRAY_BUFFER, vbo_size * sizeof(double) * 2, vbo, GL_DYNAMIC_DRAW);

    glUseProgram(m_shader);
    glBindVertexArray(m_vao);

    glDrawArrays(GL_POINTS, 0, vbo_size);

    glBindVertexArray(0);
    glUseProgram(0);

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void Render::shutdown()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Render::createShader()
{

    std::string vertex_shader_source = readShaderSource("./assets/shader.vert");
    std::string fragment_shader_source = readShaderSource("./assets/shader.frag");

    const GLchar *vertex_shader = vertex_shader_source.c_str();
    const GLchar *fragment_shader = fragment_shader_source.c_str();

    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(VertexShaderID, 1, &vertex_shader, NULL);
    glCompileShader(VertexShaderID);

    glShaderSource(FragmentShaderID, 1, &fragment_shader, NULL);
    glCompileShader(FragmentShaderID);

    m_shader = glCreateProgram();
    glAttachShader(m_shader, VertexShaderID);
    glAttachShader(m_shader, FragmentShaderID);
    glLinkProgram(m_shader);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
}

void Render::setUpBuffers(int vbo_size, double *masses)
{
    glGenBuffers(1, &m_points_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_points_vbo);
    glBufferData(GL_ARRAY_BUFFER, vbo_size * 2 * sizeof(GL_DOUBLE), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &m_masses_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_masses_vbo);
    glBufferData(GL_ARRAY_BUFFER, vbo_size * sizeof(GL_DOUBLE), masses, GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_points_vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_masses_vbo);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_DOUBLE, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}