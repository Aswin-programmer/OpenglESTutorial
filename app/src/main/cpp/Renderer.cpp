#include "Renderer.h"


#include "Renderer.h"
#include <GLES3/gl3.h>

constexpr auto VERT_CODE = R"(
#version 300 es
precision mediump float;
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

out vec4 color;

uniform mat4 projection;
uniform mat4 model;

void main()
{
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0f);
    color = vec4(aColor, 1.0);
}
)";
constexpr auto FRAG_CODE = R"(
#version 300 es
precision mediump float;

in vec4 color;

out vec4 frag_color;

uniform vec3 uniform_color;

void main()
{
    frag_color = vec4(uniform_color, 1.0);
}
)";

Renderer::Renderer(android_app *app)
    :
        color_test{COLOR(1, 1, 1)}
{
    if (!app || !app->window) {
        LOG_ERROR("Renderer received null app or window!");
        return;
    }

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        LOG_ERROR("Failed to get EGL display: 0x%x", eglGetError());
        return;
    }

    if (eglInitialize(display, nullptr, nullptr) == EGL_FALSE) {
        LOG_ERROR("Failed to initialize EGL: 0x%x", eglGetError());
        return;
    }

    EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_NONE
    };

    EGLint num_configs;
    if (!eglChooseConfig(display, attribs, &config, 1, &num_configs) || num_configs < 1) {
        LOG_ERROR("eglChooseConfig failed: 0x%x", eglGetError());
        return;
    }

    surface = eglCreateWindowSurface(display, config, app->window, nullptr);
    if (surface == EGL_NO_SURFACE) {
        LOG_ERROR("Failed to create EGL window surface: 0x%x", eglGetError());
        return;
    }

    EGLint opengl_context_attribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 3,
            EGL_NONE
    };
    context = eglCreateContext(display, config, nullptr, opengl_context_attribs);
    if (context == EGL_NO_CONTEXT) {
        LOG_ERROR("Failed to create EGL context: 0x%x", eglGetError());
        return;
    }

    if (!eglMakeCurrent(display, surface, surface, context)) {
        LOG_ERROR("eglMakeCurrent failed: 0x%x", eglGetError());
        return;
    }

    LOG_INFO("EGL Initialization is completed!");

    // Vertex format: x, y, r, g, b
    float vertices[] = {
            // x, y, r, g, b
            128.0f, 128.0f, 1.0f, 0.0f, 0.0f, // top-left
            256.0f, 128.0f, 0.0f, 1.0f, 0.0f, // top-right
            256.0f, 256.0f, 0.0f, 0.0f, 1.0f, // bottom-right
            128.0f, 256.0f, 1.0f, 1.0f, 0.0f  // bottom-left
    };

    unsigned int indices[] = {
            0, 1, 2,   // First triangle (top-right)
            2, 3, 0    // Second triangle (bottom-left)
    };

    // VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // EBO
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);

    auto compile_shader = [](GLenum type, const char* src) -> GLuint {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            char info_log[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, info_log);
            LOG_ERROR("Failed to compile the shader code: %s", info_log);
        }

        return shader;
    };

    GLuint vert = compile_shader(GL_VERTEX_SHADER, VERT_CODE);
    GLuint frag = compile_shader(GL_FRAGMENT_SHADER, FRAG_CODE);

    program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success)
    {
        char info_log[1024];
        glGetProgramInfoLog(program, 1024, nullptr, info_log);
        LOG_ERROR("Failed to compile the link shader: %s", info_log);
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    glUseProgram(program);
    glUniform3f(glGetUniformLocation(program, "uniform_color"), color_test.r, color_test.g, color_test.b);

    projection_location = glGetUniformLocation(program, "projection");
    model_location = glGetUniformLocation(program, "model");
}

Renderer::~Renderer() {
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);

    if (display != EGL_NO_DISPLAY) {
        if (context != EGL_NO_CONTEXT)
            eglDestroyContext(display, context);

        if (surface != EGL_NO_SURFACE)
            eglDestroySurface(display, surface);

        eglTerminate(display);
    }

    context = EGL_NO_CONTEXT;
    surface = EGL_NO_SURFACE;
    display = EGL_NO_DISPLAY;
}

void Renderer::Do_Frame() {
    int width, height;
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    glViewport(0, 0, width, height);
    glClearColor(1.f, 1.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);

    float screenPortWidth = 1280;
    float screenPortHeight = 720;
    glm::mat4 projection = glm::ortho(
                0.0f,
                screenPortWidth,
                screenPortHeight,
                0.0f,
                -1.0f,
                +1.0f
            );
    glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm::value_ptr(projection));

    glm::mat4 model = glm::mat4(1.0f);
    const float centerX = 192.0f;
    const float centerY = 192.0f;

    auto now = std::chrono::high_resolution_clock::now();
    double timeInSeconds = std::chrono::duration<double>(now.time_since_epoch()).count();
    float angle = glm::radians((float)fmod(timeInSeconds * 90.0, 360.0)); // slower rotation

    model = glm::translate(model, glm::vec3(centerX, centerY, 0.0f));   // Move to center
    model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));     // Rotate
    model = glm::translate(model, glm::vec3(-centerX, -centerY, 0.0f)); // Move back
    glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));

    glUniform3f(glGetUniformLocation(program, "uniform_color"), 0, color_test.g, 0);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    auto res = eglSwapBuffers(display, surface);
    assert(res);
}
