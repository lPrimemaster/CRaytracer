#include "../inc/display_acc.h"

static const char* vertex_shader_text =
"#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"layout (location = 1) in vec2 uvs;\n"
"out vec2 TexCoord;\n"

"void main()\n"
"{\n"
"   TexCoord = uvs;\n"
"   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
"}\n";

static const char* fragment_shader_text =
"#version 330 core\n"
"uniform sampler2D tex;\n"
"out vec4 FragColor;\n"
"in vec2 TexCoord;\n"

"void main()\n"
"{\n"
"   FragColor = texture(tex, TexCoord);\n"
"}\n";

GLuint create_shader(i32 shader_type, const char* source)
{
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int  success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("SHADER COMPILATION_FAILED\n%s\n", infoLog);
    }
    return shader;
}

GLuint create_program()
{
    GLuint vertex_shader = create_shader(GL_VERTEX_SHADER, vertex_shader_text);
    GLuint fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_shader_text);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    int  success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("SHADER LINK_FAILED\n%s\n", infoLog);
    }

    glUseProgram(program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

typedef struct _acc_init_data
{
    GLuint texture;

    GLuint rect_vao;
    GLuint rect_vbo;
} acc_init_data;

acc_init_data init(i32 w, i32 h)
{
    acc_init_data r;
    glGenTextures(1, &r.texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r.texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

    float vertices[] = {
        -1.0f, -1.0f, 0.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &r.rect_vao);
    glBindVertexArray(r.rect_vao);
    glGenBuffers(1, &r.rect_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, r.rect_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return r;
}

void destroy(acc_init_data r)
{
    glDeleteTextures(1, &r.texture);
    glDeleteVertexArrays(1, &r.rect_vao);
    glDeleteBuffers(1, &r.rect_vbo);
}

void window_loop(ray_dispatcher* rd, i32 width, i32 height)
{
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(width, height, "Raytracer", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader(glfwGetProcAddress);
    glfwSwapInterval(1);

    GLuint program = create_program();
    acc_init_data data = init(width, height);

    while (!glfwWindowShouldClose(window))
    {
        EnterCriticalSection(&CriticalSection);
        // TODO: Maybe consider using a PBO later on for performance
        glBindTexture(GL_TEXTURE_2D, data.texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, rd->image->data);
        LeaveCriticalSection(&CriticalSection);

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);
        glBindVertexArray(data.rect_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    _InterlockedExchange16(&rd->atomic_window_running, 0);

    destroy(data);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();
}