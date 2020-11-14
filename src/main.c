#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define GL_GLEXT_PROTOTYPES

#include <GLFW/glfw3.h>

typedef FILE File;

typedef uint8_t  u8;
typedef uint32_t u32;

typedef int32_t i32;

typedef float  f32;
typedef double f64;

typedef enum {
    FALSE = 0,
    TRUE = 1,
} Bool;

typedef struct {
    f64 start;
    u8  count;
} Fps;

typedef struct {
    f64 start;
    f64 step;
    f64 elapsed;
} Frame;

#define SIZE_BUFFER 512

typedef struct {
    char buffer[SIZE_BUFFER];
} Memory;

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480

#define MICROSECONDS 1000000.0f

static const f64 FRAME_DURATION = (1.0f / 60.0f) * MICROSECONDS;
static u32       VBO;
static u32       VAO;

// clang-format off
static const f32 VERTICES[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f,
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
    return program;
}

static void set_objects(void) {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, FALSE, 3 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);
}

static void update(GLFWwindow* window) {
    glfwPollEvents();
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, TRUE);
    }
}

static void draw(GLFWwindow* window) {
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glfwSwapBuffers(window);
}

static void loop(GLFWwindow* window) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    Frame frame = {0};
    Fps   fps = {0};
    printf("\n");
    while (!glfwWindowShouldClose(window)) {
        frame.start = glfwGetTime() * MICROSECONDS;
        update(window);
        draw(window);
        frame.step = glfwGetTime() * MICROSECONDS;
        frame.elapsed = (frame.step - frame.start);
        if (frame.elapsed < FRAME_DURATION) {
            usleep((u32)(FRAME_DURATION - frame.elapsed));
        }
        if (++fps.count == 30) {
            printf("\033[1A%10.4f fps\n",
                   fps.count / (frame.step - fps.start) * MICROSECONDS);
            fps.start = frame.start;
            fps.count = 0;
        }
    }
}

static void error_callback(i32 code, const char* error) {
    fprintf(stderr, "%d: %s\n", code, error);
    exit(EXIT_FAILURE);
}

i32 main(i32 n, const char** args) {
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
    GLFWwindow* window = get_window("main");
    u32         program = get_program(memory,
                              get_shader(memory, args[1], GL_VERTEX_SHADER),
                              get_shader(memory, args[2], GL_FRAGMENT_SHADER));
    set_objects();
    glUseProgram(program);
    loop(window);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);
    glfwTerminate();
    free(memory);
    return EXIT_SUCCESS;
}
