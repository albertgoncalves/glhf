#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gl.h"
#include "math.h"

typedef struct {
    f32 start;
    f32 step;
    f32 elapsed;
    f32 fps_start;
    u8  fps_count;
} Frame;

typedef struct {
    f32 time;
} State;

typedef struct {
    i32 time;
    i32 model;
    i32 view;
    i32 projection;
    i32 transform;
} Uniform;

#define MICROSECONDS 1000000.0f

static const f32 FRAME_DURATION = (1.0f / 60.0f) * MICROSECONDS;

static Mat4 MODEL;
// NOTE: "Up" orientation of the object.
static const Vec3 MODEL_AXIS = {
    .x = 1.0f,
    .y = 0.0f,
    .z = 0.0f,
};
static Mat4 VIEW;
// NOTE: Position of the view plane?
static const Vec3 VIEW_TRANSLATION = {
    .x = 0.0f,
    .y = 0.0f,
    .z = -3.0f,
};
static Mat4 PROJECTION;

static Mat4       TRANSFORM;
static const Vec3 TRANSFORM_AXIS = {
    .x = 0.0f,
    .y = 0.0f,
    .z = 1.0f,
};
static const Vec3 TRANSFORM_SCALE = {
    .x = 1.5f,
    .y = 1.5f,
    .z = 1.5f,
};

static void set_input(GLFWwindow* window) {
    glfwPollEvents();
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, TRUE);
    }
}

static void draw(GLFWwindow* window) {
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glfwSwapBuffers(window);
}

static void loop(GLFWwindow* window, u32 program) {
    State   state = {0};
    Frame   frame = {0};
    Uniform uniform = {
        .time = glGetUniformLocation(program, "U_TIME"),
        .model = glGetUniformLocation(program, "U_MODEL"),
        .view = glGetUniformLocation(program, "U_VIEW"),
        .projection = glGetUniformLocation(program, "U_PROJECTION"),
        .transform = glGetUniformLocation(program, "U_TRANSFORM"),
    };
    MODEL = rotate_mat4(get_radians(-55.0f), MODEL_AXIS);
    VIEW = translate_mat4(VIEW_TRANSLATION);
    PROJECTION = perspective_mat4(get_radians(45.0f),
                                  (f32)WINDOW_WIDTH / (f32)WINDOW_HEIGHT,
                                  0.1f,
                                  100.0f);
    glUniformMatrix4fv(uniform.model, 1, FALSE, &MODEL.cell[0][0]);
    glUniformMatrix4fv(uniform.view, 1, FALSE, &VIEW.cell[0][0]);
    glUniformMatrix4fv(uniform.projection, 1, FALSE, &PROJECTION.cell[0][0]);
    glClearColor(0.175f, 0.175f, 0.175f, 1.0f);
    printf("\n");
    while (!glfwWindowShouldClose(window)) {
        state.time = (f32)glfwGetTime();
        frame.start = state.time * MICROSECONDS;
        set_input(window);
        glUniform1f(uniform.time, state.time);
        TRANSFORM = mul_mat4_mat4(
            rotate_mat4(get_radians((f32)state.time * 25.0f), TRANSFORM_AXIS),
            scale_mat4(TRANSFORM_SCALE));
        glUniformMatrix4fv(uniform.transform, 1, FALSE, &TRANSFORM.cell[0][0]);
        draw(window);
        frame.step = (f32)glfwGetTime() * MICROSECONDS;
        frame.elapsed = (frame.step - frame.start);
        if (frame.elapsed < FRAME_DURATION) {
            usleep((u32)(FRAME_DURATION - frame.elapsed));
        }
        if (++frame.fps_count == 30) {
            printf("\033[1A%10.4f fps\n",
                   frame.fps_count / (frame.step - frame.fps_start) *
                       MICROSECONDS);
            frame.fps_start = frame.start;
            frame.fps_count = 0;
        }
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
           "sizeof(Uniform)        : %zu\n"
           "sizeof(State)          : %zu\n"
           "sizeof(Memory)         : %zu\n"
           "sizeof(memory->buffer) : %zu\n\n",
           sizeof(Bool),
           sizeof(Vec3),
           sizeof(Mat4),
           sizeof(Frame),
           sizeof(Uniform),
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
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(program);
    glfwTerminate();
    free(memory);
    return EXIT_SUCCESS;
}
