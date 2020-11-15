#ifndef __GL_H__
#define __GL_H__

#define GL_GLEXT_PROTOTYPES

#include <GLFW/glfw3.h>

#include "memory.h"

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480

static u32 VBO;
static u32 VAO;
// static u32 EBO;

// clang-format off
static const f32 VERTICES[] = {
    // NOTE: (x,y,z)        // NOTE: (r,g,b)
    -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,    0.0f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,    0.0f, 1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,    1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,    1.0f, 0.0f, 0.0f,

     0.5f,  0.5f,  0.5f,    1.0f, 0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,    1.0f, 0.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,    1.0f, 1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,    0.0f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,    1.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,    1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,    0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,    0.0f, 1.0f, 1.0f,
};
// static const i32 INDICES[] = {
//     0, 1, 3,
//     1, 2, 3,
// };
// clang-format on

static void framebuffer_size_callback(GLFWwindow* _, i32 width, i32 height) {
    glViewport(0, 0, width, height);
}

static GLFWwindow* get_window(const char* name) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window =
        glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, name, NULL, NULL);
    if (!window) {
        glfwTerminate();
        ERROR("!window");
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowAspectRatio(window, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSwapInterval(1);
    return window;
}

static u32 get_shader(Memory* memory, const char* filename, GLenum type) {
    set_file(memory, filename);
    u32         shader = glCreateShader(type);
    const char* source = memory->buffer;
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    i32 status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        memset(memory->buffer, 0, sizeof(memory->buffer));
        glGetShaderInfoLog(shader,
                           sizeof(memory->buffer),
                           NULL,
                           memory->buffer);
        ERROR(memory->buffer);
    }
    return shader;
}

static u32 get_program(Memory* memory,
                       u32     vertex_shader,
                       u32     fragment_shader) {
    u32 program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    i32 status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        memset(memory->buffer, 0, sizeof(memory->buffer));
        glGetProgramInfoLog(program,
                            sizeof(memory->buffer),
                            NULL,
                            memory->buffer);
        ERROR(memory->buffer);
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glUseProgram(program);
    return program;
}

static void set_objects(void) {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
    // glGenBuffers(1, &EBO);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER,
    //              sizeof(INDICES),
    //              INDICES,
    //              GL_STATIC_DRAW);
    i32 stride = 6 * sizeof(f32);
    {
        // NOTE: Position attribute.
        u32   index = 0;
        void* offset = (void*)0;
        glVertexAttribPointer(index, 3, GL_FLOAT, FALSE, stride, offset);
        glEnableVertexAttribArray(index);
    }
    {
        // NOTE: Color attribute.
        u32   index = 1;
        void* offset = (void*)(3 * sizeof(f32));
        glVertexAttribPointer(index, 3, GL_FLOAT, FALSE, stride, offset);
        glEnableVertexAttribArray(index);
    }
    glEnable(GL_DEPTH_TEST);
}

static void draw(GLFWwindow* window) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glfwSwapBuffers(window);
}

#endif
