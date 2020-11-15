#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define GL_GLEXT_PROTOTYPES

#include <GLFW/glfw3.h>

#include "prelude.h"

typedef struct {
    f32 start;
    f32 step;
    f32 elapsed;
    f32 fps_start;
    u8  fps_count;
} Frame;

typedef struct {
    i32 time;
} Uniform;

typedef struct {
    f32 time;
} State;

#define SIZE_BUFFER 512

typedef struct {
    char buffer[SIZE_BUFFER];
} Memory;

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480

#define MICROSECONDS 1000000.0f

static const f32 FRAME_DURATION = (1.0f / 60.0f) * MICROSECONDS;
static u32       VBO;
static u32       VAO;

// clang-format off
static const f32 VERTICES[] = {
    // positions            // colors
     0.5f, -0.5f, 0.0f,     1.0f, 0.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,   // bottom left
     0.0f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f    // top
};
// clang-format on

#define ERROR(x)                                                           \
    {                                                                      \
        fprintf(stderr, "%s:%s:%d %s\n", __FILE__, __func__, __LINE__, x); \
        exit(EXIT_FAILURE);                                                \
    }

static void set_file(Memory* memory, const char* filename) {
    File* file = fopen(filename, "r");
    if (!file) {
        ERROR("Unable to open file");
    }
    fseek(file, 0, SEEK_END);
    u32 file_size = (u32)ftell(file);
    if (sizeof(memory->buffer) <= file_size) {
        ERROR("sizeof(memory->buffer) <= file_size");
    }
    rewind(file);
    memory->buffer[file_size] = '\0';
    if (fread(&memory->buffer, sizeof(char), file_size, file) != file_size) {
        ERROR("`fread` failed");
    }
    fclose(file);
}

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
}

static void update(GLFWwindow* window) {
    glfwPollEvents();
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, TRUE);
    }
}

static void draw(GLFWwindow* window, State state, Uniform uniform) {
    glUniform1f(uniform.time, state.time);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glfwSwapBuffers(window);
}

static void loop(GLFWwindow* window, Uniform uniform) {
    State state = {0};
    Frame frame = {0};
    glClearColor(0.175f, 0.175f, 0.175f, 1.0f);
    printf("\n");
    while (!glfwWindowShouldClose(window)) {
        state.time = (f32)glfwGetTime();
        frame.start = state.time * MICROSECONDS;
        update(window);
        draw(window, state, uniform);
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
    printf("sizeof(Frame)   : %zu\n"
           "sizeof(Uniform) : %zu\n"
           "sizeof(State)   : %zu\n"
           "sizeof(Memory)  : %zu\n\n",
           sizeof(Frame),
           sizeof(Uniform),
           sizeof(State),
           sizeof(Memory));
    if (n < 3) {
        ERROR("Missing args");
    }
    Memory* memory = calloc(1, sizeof(Memory));
    if (!memory) {
        ERROR("`calloc` failed");
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
    Uniform uniform = {
        .time = glGetUniformLocation(program, "U_TIME"),
    };
    loop(window, uniform);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);
    glfwTerminate();
    free(memory);
    return EXIT_SUCCESS;
}
