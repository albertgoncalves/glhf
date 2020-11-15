#ifndef __SCENE_H__
#define __SCENE_H__

#include "init.h"
#include "math.h"

typedef struct {
    i32 time;
    i32 model;
    i32 view;
    i32 projection;
    i32 transform;
} Uniforms;

typedef struct {
    f32 time;
} State;

// clang-format off
static const f32 POSITIONS_COLORS[] = {
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

    -0.5f,  0.5f,  0.5f,    0.0f, 1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,    0.0f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,    0.0f, 1.0f, 1.0f,

     0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 1.0f,

    -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 0.0f,

    -0.5f,  0.5f, -0.5f,    0.0f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,    1.0f, 1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,    0.0f, 1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,    0.0f, 1.0f, 1.0f,
};
static const f32 COORDS[] = {
    -4.5f,
    -1.5f,
     1.5f,
     4.5f,
};
// clang-format on

static Mat4 TRANSLATIONS[16];

static Mat4       MODEL;
static const f32  MODEL_DEGREES = 15.0f;
static const Vec3 MODEL_AXIS = {
    .x = 0.0f,
    .y = 0.0f,
    .z = 1.0f,
};
static const Vec3 MODEL_SCALE = {
    .x = 1.0f,
    .y = 1.0f,
    .z = 1.0f,
};

static Mat4       VIEW;
static const Vec3 VIEW_EYE = {
    .x = 7.5f,
    .y = 7.5f,
    .z = -12.5f, // NOTE: Forward-and-back distance to object.
};
static const Vec3 VIEW_TARGET = {
    .x = 0.0f,
    .y = 0.0f,
    .z = 0.0f,
};
static const Vec3 VIEW_UP = {
    .x = 0.0f, // NOTE: `x`-axis is left/right.
    .y = 1.0f, // NOTE: `y`-axis is up/down.
    .z = 0.0f, // NOTE: `z`-axis is forward/back.
};

static Mat4 PROJECTION;

static Mat4       TRANSFORM;
static const Vec3 TRANSFORM_AXIS = {
    .x = 0.0f,
    .y = 1.0f,
    .z = 0.0f,
};

static u32 VAO;
static u32 VBO;
static u32 IVBO;

static void set_vertex_attrib(u32 index, i32 size, i32 stride, void* offset) {
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, offset);
}

static void set_objects(void) {
    u32 index = 0;
    {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
    }
    {
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(POSITIONS_COLORS),
                     POSITIONS_COLORS,
                     GL_STATIC_DRAW);
        i32 position_width = 3;
        i32 color_width = 3;
        i32 stride = (i32)sizeof(f32) * (position_width + color_width);
        set_vertex_attrib(index++, position_width, stride, (void*)0);
        set_vertex_attrib(index++,
                          color_width,
                          stride,
                          (void*)(sizeof(f32) * (usize)position_width));
    }
    {
        {
            u8 k = 0;
            for (u8 i = 0; i < 4; ++i) {
                for (u8 j = 0; j < 4; ++j) {
                    Vec3 position = {
                        .x = COORDS[i],
                        .y = 0.0f,
                        .z = COORDS[j],
                    };
                    f32  size = 1.0f / sqrtf((f32)k + 1.0f);
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
        glGenBuffers(1, &IVBO);
        glBindBuffer(GL_ARRAY_BUFFER, IVBO);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(TRANSLATIONS),
                     &TRANSLATIONS[0].cell[0][0],
                     GL_STATIC_DRAW);
        i32 stride = sizeof(TRANSLATIONS[0]);
        // NOTE: Instances are limited to `sizeof(f32) * 4`, so `Mat4` data
        // must be constructed in four parts.
        usize offset = sizeof(f32) * 4;
        for (usize i = 0; i < 4; ++i) {
            set_vertex_attrib(index, 4, stride, (void*)(i * offset));
            glVertexAttribDivisor(index++, 1);
        }
    }
    glEnable(GL_DEPTH_TEST);
}

static Uniforms get_uniforms(u32 program) {
    Uniforms uniforms = {
        .time = glGetUniformLocation(program, "U_TIME"),
        .model = glGetUniformLocation(program, "U_MODEL"),
        .view = glGetUniformLocation(program, "U_VIEW"),
        .projection = glGetUniformLocation(program, "U_PROJECTION"),
        .transform = glGetUniformLocation(program, "U_TRANSFORM"),
    };
    return uniforms;
}

static void set_static_uniforms(Uniforms uniforms) {
    MODEL = mul_mat4(rotate_mat4(get_radians(MODEL_DEGREES), MODEL_AXIS),
                     scale_mat4(MODEL_SCALE));
    glUniformMatrix4fv(uniforms.model, 1, FALSE, &MODEL.cell[0][0]);
    VIEW = look_at_mat4(VIEW_EYE, VIEW_TARGET, VIEW_UP);
    glUniformMatrix4fv(uniforms.view, 1, FALSE, &VIEW.cell[0][0]);
    PROJECTION = perspective_mat4(get_radians(45.0f),
                                  (f32)WINDOW_WIDTH / (f32)WINDOW_HEIGHT,
                                  0.1f,
                                  100.0f);
    glUniformMatrix4fv(uniforms.projection, 1, FALSE, &PROJECTION.cell[0][0]);
}

static void set_dynamic_uniforms(Uniforms uniforms, State state) {
    glUniform1f(uniforms.time, state.time);
    TRANSFORM =
        rotate_mat4(get_radians((f32)state.time * 25.0f), TRANSFORM_AXIS);
    glUniformMatrix4fv(uniforms.transform, 1, FALSE, &TRANSFORM.cell[0][0]);
}

static void set_state(State* state) {
    state->time = (f32)glfwGetTime();
}

static void draw(GLFWwindow* window) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArraysInstanced(GL_TRIANGLES,
                          0,
                          36,
                          sizeof(TRANSLATIONS) / sizeof(TRANSLATIONS[0]));
    glfwSwapBuffers(window);
}

#endif
