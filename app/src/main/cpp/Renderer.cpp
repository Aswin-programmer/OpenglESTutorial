#include "Renderer.h"


#include "Renderer.h"
#include <GLES3/gl3.h>

Renderer::Renderer(android_app *app) {
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
}

Renderer::~Renderer() {
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

    auto res = eglSwapBuffers(display, surface);
    assert(res);
}
