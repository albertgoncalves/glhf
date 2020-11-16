#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "scene.h"

typedef struct {
    f32 start;
    f32 step;
    f32 elapsed;
    f32 fps_start;
    u8  fps_count;
} Frame;

#define MICROSECONDS 1000000.0f

static const f32 FRAME_DURATION = (1.0f / 60.0f) * MICROSECONDS;

static void set_input(GLFWwindow* window) {
    glfwPollEvents();
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, TRUE);
    }
}

static void set_frame(Frame* frame) {
    frame->step = (f32)glfwGetTime() * MICROSECONDS;
    frame->elapsed = (frame->step - frame->start);
    if (frame->elapsed < FRAME_DURATION) {
        usleep((u32)(FRAME_DURATION - frame->elapsed));
    }
    if (++frame->fps_count == 30) {
        printf("\033[1A%10.4f fps\n",
               frame->fps_count / (frame->step - frame->fps_start) *
                   MICROSECONDS);
        frame->fps_start = frame->start;
        frame->fps_count = 0;
    }
}

static void loop(GLFWwindow* window, u32 program) {
    State    state = {0};
    Frame    frame = {0};
    Uniforms uniforms = get_uniforms(program);
    set_static_uniforms(uniforms);
    set_draw();
    printf("\n");
    while (!glfwWindowShouldClose(window)) {
        set_state(&state);
        set_input(window);
        set_dynamic_uniforms(uniforms, state);
        draw(window);
        frame.start = state.time * MICROSECONDS;
        set_frame(&frame);
    }
}

static void error_callback(i32 code, const char* error) {
    fprintf(stderr, "%d: %s\n", code, error);
    exit(EXIT_FAILURE);
}

i32 main(i32 n, const char** args) {
    Memory* memory = calloc(1, sizeof(Memory));
    if (!memory) {
        ERROR("`calloc` failed");
    }
    printf("sizeof(Bool)           : %zu\n"
           "sizeof(Vec3)           : %zu\n"
           "sizeof(Mat4)           : %zu\n"
           "sizeof(Frame)          : %zu\n"
           "sizeof(Uniforms)       : %zu\n"
           "sizeof(State)          : %zu\n"
           "sizeof(Memory)         : %zu\n"
           "sizeof(memory->buffer) : %zu\n\n",
           sizeof(Bool),
           sizeof(Vec3),
           sizeof(Mat4),
           sizeof(Frame),
           sizeof(Uniforms),
           sizeof(State),
           sizeof(Memory),
           sizeof(memory->buffer));
    if (n < 3) {
        ERROR("Missing args");
    }
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        ERROR("!glfwInit()");
    }
    GLFWwindow* window = get_window("float");
    u32         program = get_program(memory,
                              get_shader(memory, args[1], GL_VERTEX_SHADER),
                              get_shader(memory, args[2], GL_FRAGMENT_SHADER));
    set_objects();
    loop(window, program);
    free_scene();
    glDeleteProgram(program);
    glfwTerminate();
    free(memory);
    return EXIT_SUCCESS;
}
