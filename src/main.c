#include "math.h"

#include <string.h>
#include <unistd.h>

#define GL_GLEXT_PROTOTYPES

#include <GLFW/glfw3.h>

// NOTE: This is a hack to hide the mouse cursor; at the moment, seems like
// `glfwSetInputMode(..., GLFW_CURSOR_DISABLED)` doesn't work as intended.
// See `https://github.com/glfw/glfw/issues/1790`.
#define GLFW_EXPOSE_NATIVE_X11

#include <GLFW/glfw3native.h>
#include <X11/extensions/Xfixes.h>

typedef struct {
    Display* display;
    Window   window;
} Native;

#define SIZE_BUFFER 1024

typedef struct {
    char buffer[SIZE_BUFFER];
} Memory;

typedef struct {
    i32 model;
    i32 projection;
    i32 time;
    i32 view;
    i32 transform;
} Uniforms;

typedef struct {
    f32 time;
} State;

typedef struct {
    f32 time;
    f32 prev;
    f32 delta;
    f32 fps_time;
    u8  fps_count;
} Frame;

#define MICROSECONDS 1000000.0f

#define FRAME_UPDATE_COUNT 10

static const f32 FRAME_DURATION = (1.0f / 60.0f) * MICROSECONDS;
static const f32 FRAME_UPDATE_STEP = FRAME_DURATION / FRAME_UPDATE_COUNT;

#define INIT_WINDOW_WIDTH  1024
#define INIT_WINDOW_HEIGHT 768

static i32 WINDOW_WIDTH = INIT_WINDOW_WIDTH;
static i32 WINDOW_HEIGHT = INIT_WINDOW_HEIGHT;

#define FBO_SCALE 4

static const i32 FBO_WIDTH = INIT_WINDOW_WIDTH / FBO_SCALE;
static const i32 FBO_HEIGHT = INIT_WINDOW_HEIGHT / FBO_SCALE;

// clang-format off
static const f32 POSITIONS_COLORS[] = {
    // NOTE: (x,y,z)        // NOTE: (r,g,b)
    -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 0.0f, //  0
     0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f, //  1
     0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f, //  2
    -0.5f,  0.5f, -0.5f,    0.0f, 1.0f, 1.0f, //  3
    -0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 0.0f, //  4
     0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f, //  5
     0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 1.0f, //  6
    -0.5f,  0.5f,  0.5f,    0.0f, 1.0f, 1.0f, //  7
};
static const usize POSITION_OFFSET = 0;
static const u32 INDICES[] = {
    0, 1, 2,
    2, 3, 0,
    4, 5, 6,
    6, 7, 4,
    7, 3, 0,
    0, 4, 7,
    6, 2, 1,
    1, 5, 6,
    0, 1, 5,
    5, 4, 0,
    3, 2, 6,
    6, 7, 3,
};
static const f32 COORDS[] = {
    -10.5f,
     -7.5f,
     -4.5f,
     -1.5f,
      1.5f,
      4.5f,
      7.5f,
     10.5f,
};
// clang-format on

static u8 COUNT_COORDS = sizeof(COORDS) / sizeof(COORDS[0]);

// NOTE: `COUNT_TRANSLATIONS == (COUNT_COORDS * COUNT_COORDS)`
#define COUNT_TRANSLATIONS 64

static Mat4 TRANSLATIONS[COUNT_TRANSLATIONS];

static Mat4       MODEL;
static const f32  MODEL_DEGREES = 15.0f;
static const Vec3 MODEL_AXIS = {
    .x = 0.0f,
    .y = 0.0f,
    .z = 1.0f,
};
static const Vec3 MODEL_SCALE = {
    .x = 1.25f,
    .y = 1.25f,
    .z = 1.25f,
};

#define VIEW_EYE_Y 0.0f

static Mat4 VIEW;
static Vec3 VIEW_EYE = {
    .x = 0.0f,
    .y = VIEW_EYE_Y,
    .z = 20.0f,
};
static Vec3 VIEW_TARGET = {
    .x = 0.0f,
    .y = 0.0f,
    .z = -1.0f,
};
static const Vec3 VIEW_UP = {
    .x = 0.0f, // NOTE: `x`-axis is left/right.
    .y = 1.0f, // NOTE: `y`-axis is up/down.
    .z = 0.0f, // NOTE: `z`-axis is forward/back.
};

#define KEY_SENSITIVITY 0.01f

static f32 CURSOR_X;
static f32 CURSOR_Y;

#define CURSOR_SENSITIVITY 0.1f

static f32 CURSOR_X_DELTA = 0.0f;
static f32 CURSOR_Y_DELTA = 0.0f;

#define VIEW_NEAR 0.1f
#define VIEW_FAR  100.0f

static f32 VIEW_YAW = -90.0f;
static f32 VIEW_PITCH = 0.0f;

#define PITCH_LIMIT 89.0f

static Mat4 PROJECTION;

static Mat4       TRANSFORM;
static const Vec3 TRANSFORM_AXIS = {
    .x = 0.0f,
    .y = 1.0f,
    .z = 0.0f,
};

static u32 VAO;
static u32 VBO;
static u32 EBO;
static u32 IBO;
static u32 FBO;
static u32 RBO;
static u32 DBO;

static const u32 INDEX_POSITION = 0;
static const u32 INDEX_COLOR = 1;
static const u32 INDEX_TRANSLATE = 2;

static void hide_cursor(Native native) {
    XFixesHideCursor(native.display, native.window);
    XFlush(native.display);
}

static void show_cursor(Native native) {
    XFixesShowCursor(native.display, native.window);
    XFlush(native.display);
}

#define CHECK_GL_ERROR()                               \
    {                                                  \
        switch (glGetError()) {                        \
        case GL_INVALID_ENUM: {                        \
            ERROR("GL_INVALID_ENUM");                  \
        }                                              \
        case GL_INVALID_VALUE: {                       \
            ERROR("GL_INVALID_VALUE");                 \
        }                                              \
        case GL_INVALID_OPERATION: {                   \
            ERROR("GL_INVALID_OPERATION");             \
        }                                              \
        case GL_INVALID_FRAMEBUFFER_OPERATION: {       \
            ERROR("GL_INVALID_FRAMEBUFFER_OPERATION"); \
        }                                              \
        case GL_OUT_OF_MEMORY: {                       \
            ERROR("GL_OUT_OF_MEMORY");                 \
        }                                              \
        case GL_NO_ERROR: {                            \
            break;                                     \
        }                                              \
        }                                              \
    }

#define NORM_CROSS(a, b) norm_vec3(cross_vec3(a, b))

static void set_input(GLFWwindow* window) {
    glfwPollEvents();
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, TRUE);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        VIEW_EYE = sub_vec3(
            VIEW_EYE,
            mul_vec3_f32(NORM_CROSS(cross_vec3(VIEW_TARGET, VIEW_UP), VIEW_UP),
                         KEY_SENSITIVITY));
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        VIEW_EYE = add_vec3(
            VIEW_EYE,
            mul_vec3_f32(NORM_CROSS(cross_vec3(VIEW_TARGET, VIEW_UP), VIEW_UP),
                         KEY_SENSITIVITY));
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        VIEW_EYE = sub_vec3(
            VIEW_EYE,
            mul_vec3_f32(NORM_CROSS(VIEW_TARGET, VIEW_UP), KEY_SENSITIVITY));
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        VIEW_EYE = add_vec3(
            VIEW_EYE,
            mul_vec3_f32(NORM_CROSS(VIEW_TARGET, VIEW_UP), KEY_SENSITIVITY));
    }
}

#define CURSOR_CALLBACK(x, y)                                            \
    {                                                                    \
        CURSOR_X = (f32)x;                                               \
        CURSOR_Y = (f32)y;                                               \
        VIEW_YAW += CURSOR_X_DELTA;                                      \
        VIEW_PITCH += CURSOR_Y_DELTA;                                    \
        if (PITCH_LIMIT < VIEW_PITCH) {                                  \
            VIEW_PITCH = PITCH_LIMIT;                                    \
        } else if (VIEW_PITCH < -PITCH_LIMIT) {                          \
            VIEW_PITCH = -PITCH_LIMIT;                                   \
        }                                                                \
        VIEW_TARGET.x =                                                  \
            cosf(get_radians(VIEW_YAW)) * cosf(get_radians(VIEW_PITCH)); \
        VIEW_TARGET.y = sinf(get_radians(VIEW_PITCH));                   \
        VIEW_TARGET.z =                                                  \
            sinf(get_radians(VIEW_YAW)) * cosf(get_radians(VIEW_PITCH)); \
        VIEW_TARGET = norm_vec3(VIEW_TARGET);                            \
    }

static void cursor_callback(GLFWwindow* _, f64 x, f64 y) {
    CURSOR_X_DELTA = ((f32)x - CURSOR_X) * CURSOR_SENSITIVITY;
    CURSOR_Y_DELTA = (CURSOR_Y - (f32)y) * CURSOR_SENSITIVITY;
    CURSOR_CALLBACK(x, y);
}

static void init_cursor_callback(GLFWwindow* window, f64 x, f64 y) {
    CURSOR_CALLBACK(x, y);
    glfwSetCursorPosCallback(window, cursor_callback);
}

static void framebuffer_size_callback(GLFWwindow* window,
                                      i32         width,
                                      i32         height) {
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
}

static GLFWwindow* get_window(const char* name) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(INIT_WINDOW_WIDTH,
                                          INIT_WINDOW_HEIGHT,
                                          name,
                                          NULL,
                                          NULL);
    if (!window) {
        glfwTerminate();
        ERROR("!window");
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowAspectRatio(window, INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT);
    // NOTE: While mouse *does* get locked to center of window, it remains
    // visible. See `https://github.com/glfw/glfw/issues/1790`.
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // NOTE: This seems to have no effect. See
    // `https://github.com/glfw/glfw/issues/1559`.
    glfwSwapInterval(1);
    return window;
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

static void set_translations(void) {
    u8 k = 0;
    for (u8 i = 0; i < COUNT_COORDS; ++i) {
        if (COUNT_TRANSLATIONS < k) {
            ERROR("COUNT_TRANSLATIONS < k");
        }
        for (u8 j = 0; j < COUNT_COORDS; ++j) {
            Vec3 position = {
                .x = COORDS[i],
                .y = -COORDS[j],
                .z = 0.0f,
            };
            f32  size = 2.0f / sqrtf((f32)k + 1.0f);
            Vec3 scale = {
                .x = size,
                .y = size,
                .z = size,
            };
            TRANSLATIONS[k++] =
                mul_mat4(translate_mat4(position), scale_mat4(scale));
        }
    }
}

static void set_vertex_attrib(u32 index, i32 size, i32 stride, void* offset) {
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, offset);
}

static void set_objects(void) {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    CHECK_GL_ERROR();
    {
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(POSITIONS_COLORS),
                     POSITIONS_COLORS,
                     GL_STATIC_DRAW);
        i32 position_width = 3;
        i32 color_width = 3;
        i32 stride = ((i32)(sizeof(f32))) * (position_width + color_width);
        set_vertex_attrib(INDEX_POSITION,
                          position_width,
                          stride,
                          (void*)POSITION_OFFSET);
        set_vertex_attrib(INDEX_COLOR,
                          color_width,
                          stride,
                          (void*)(sizeof(f32) * (usize)position_width));
        CHECK_GL_ERROR();
    }
    {
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(INDICES),
                     INDICES,
                     GL_STATIC_DRAW);
        CHECK_GL_ERROR();
    }
    {
        set_translations();
        glGenBuffers(1, &IBO);
        glBindBuffer(GL_ARRAY_BUFFER, IBO);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(TRANSLATIONS),
                     &TRANSLATIONS[0].cell[0][0],
                     GL_STATIC_DRAW);
        i32 stride = sizeof(TRANSLATIONS[0]);
        // NOTE: Instances are limited to `sizeof(f32) * 4`, so `Mat4` data
        // must be constructed in four parts.
        usize offset = sizeof(f32) * 4;
        for (u32 i = 0; i < 4; ++i) {
            u32 index = INDEX_TRANSLATE + i;
            set_vertex_attrib(index, 4, stride, (void*)(i * offset));
            glVertexAttribDivisor(index, 1);
        }
        CHECK_GL_ERROR();
    }
    {
        glGenRenderbuffers(1, &RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, FBO_WIDTH, FBO_HEIGHT);
        CHECK_GL_ERROR();
    }
    {
        glGenRenderbuffers(1, &DBO);
        glBindRenderbuffer(GL_RENDERBUFFER, DBO);
        glRenderbufferStorage(GL_RENDERBUFFER,
                              GL_DEPTH_COMPONENT,
                              FBO_WIDTH,
                              FBO_HEIGHT);
        CHECK_GL_ERROR();
    }
    {
        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                  GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER,
                                  RBO);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                  GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER,
                                  DBO);
        CHECK_GL_ERROR();
    }
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        CHECK_GL_ERROR();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    CHECK_GL_ERROR();
}

static Uniforms get_uniforms(u32 program) {
    Uniforms uniforms = {
        .model = glGetUniformLocation(program, "U_MODEL"),
        .projection = glGetUniformLocation(program, "U_PROJECTION"),
        .time = glGetUniformLocation(program, "U_TIME"),
        .view = glGetUniformLocation(program, "U_VIEW"),
        .transform = glGetUniformLocation(program, "U_TRANSFORM"),
    };
    return uniforms;
}

static void set_static_uniforms(Uniforms uniforms) {
    MODEL = mul_mat4(rotate_mat4(get_radians(MODEL_DEGREES), MODEL_AXIS),
                     scale_mat4(MODEL_SCALE));
    glUniformMatrix4fv(uniforms.model, 1, FALSE, &MODEL.cell[0][0]);
    CHECK_GL_ERROR();
}

static void set_dynamic_uniforms(Uniforms uniforms, State state) {
    PROJECTION = perspective_mat4(get_radians(45.0f),
                                  (f32)WINDOW_WIDTH / (f32)WINDOW_HEIGHT,
                                  VIEW_NEAR,
                                  VIEW_FAR);
    glUniformMatrix4fv(uniforms.projection, 1, FALSE, &PROJECTION.cell[0][0]);
    glUniform1f(uniforms.time, state.time);
    VIEW = look_at_mat4(VIEW_EYE, add_vec3(VIEW_EYE, VIEW_TARGET), VIEW_UP);
    glUniformMatrix4fv(uniforms.view, 1, FALSE, &VIEW.cell[0][0]);
    TRANSFORM =
        rotate_mat4(get_radians((f32)state.time * 25.0f), TRANSFORM_AXIS);
    glUniformMatrix4fv(uniforms.transform, 1, FALSE, &TRANSFORM.cell[0][0]);
    CHECK_GL_ERROR();
}

static void draw(GLFWwindow* window) {
    {
        // NOTE: Bind off-screen render target.
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glViewport(0, 0, FBO_WIDTH, FBO_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    {
        // NOTE: Draw scene.
        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES,
                                sizeof(INDICES) / sizeof(INDICES[0]),
                                GL_UNSIGNED_INT,
                                (void*)POSITION_OFFSET,
                                COUNT_TRANSLATIONS);
    }
    {
        // NOTE: Blit off-screen to on-screen.
        glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glBlitFramebuffer(0,
                          0,
                          FBO_WIDTH,
                          FBO_HEIGHT,
                          0,
                          0,
                          WINDOW_WIDTH,
                          WINDOW_HEIGHT,
                          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                          GL_NEAREST);
    }
    glfwSwapBuffers(window);
}

static void set_frame(Frame* frame) {
    f32 now = (f32)glfwGetTime() * MICROSECONDS;
    f32 elapsed = (now - frame->time);
    if (elapsed < FRAME_DURATION) {
        usleep((u32)(FRAME_DURATION - elapsed));
    }
    if (++frame->fps_count == 30) {
        printf("\033[4A"
               "fps    :%8.2f\n"
               "eye    :%8.2f%8.2f%8.2f\n"
               "target :%8.2f%8.2f%8.2f\n"
               "up     :%8.2f%8.2f%8.2f\n",
               (frame->fps_count / (now - frame->fps_time)) * MICROSECONDS,
               VIEW_EYE.x,
               VIEW_EYE.y,
               VIEW_EYE.z,
               VIEW_TARGET.x,
               VIEW_TARGET.y,
               VIEW_TARGET.z,
               VIEW_UP.x,
               VIEW_UP.y,
               VIEW_UP.z);
        frame->fps_time = frame->time;
        frame->fps_count = 0;
    }
    frame->prev = frame->time;
}

static void loop(GLFWwindow* window, u32 program) {
    State    state = {0};
    Frame    frame = {0};
    Uniforms uniforms = get_uniforms(program);
    set_static_uniforms(uniforms);
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    printf("\n\n\n\n");
    while (!glfwWindowShouldClose(window)) {
        state.time = (f32)glfwGetTime();
        frame.time = state.time * MICROSECONDS;
        frame.delta += frame.time - frame.prev;
        while (FRAME_UPDATE_STEP < frame.delta) {
            set_input(window);
            frame.delta -= FRAME_UPDATE_STEP;
        }
        set_dynamic_uniforms(uniforms, state);
        draw(window);
        set_frame(&frame);
    }
}

static void error_callback(i32 code, const char* error) {
    fprintf(stderr, "%d: %s\n", code, error);
    exit(EXIT_FAILURE);
}

i32 main(i32 n, const char** args) {
    printf("GLFW version: %s\n\n", glfwGetVersionString());
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
    Native native = {
        .display = glfwGetX11Display(),
        .window = glfwGetX11Window(window),
    };
    hide_cursor(native);
    glfwSetCursorPosCallback(window, init_cursor_callback);
    loop(window, program);
    show_cursor(native);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &IBO);
    glDeleteFramebuffers(1, &FBO);
    glDeleteRenderbuffers(1, &RBO);
    glDeleteRenderbuffers(1, &DBO);
    glDeleteProgram(program);
    glfwTerminate();
    free(memory);
    return EXIT_SUCCESS;
}
