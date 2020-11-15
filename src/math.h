#ifndef __MATH_H__
#define __MATH_H__

#include <immintrin.h>
#include <math.h>

#include "prelude.h"

#define PI 3.1415926535897932385f

typedef struct {
    f32 x;
    f32 y;
    f32 z;
} Vec3;

typedef __m128 Simd4F32;

typedef union {
    f32      cell[4][4];
    Simd4F32 column[4];
} Mat4;

static f32 get_radians(f32 degrees) {
    return (degrees * PI) / 180.0f;
}

static f32 get_degrees(f32 radians) {
    return (radians * 180.0f) / PI;
}

static Simd4F32 linear_combine(Simd4F32 l, Mat4 r) {
    Simd4F32 out;
    out = _mm_mul_ps(_mm_shuffle_ps(l, l, 0x00), r.column[0]);
    out = _mm_add_ps(out, _mm_mul_ps(_mm_shuffle_ps(l, l, 0x55), r.column[1]));
    out = _mm_add_ps(out, _mm_mul_ps(_mm_shuffle_ps(l, l, 0xaa), r.column[2]));
    out = _mm_add_ps(out, _mm_mul_ps(_mm_shuffle_ps(l, l, 0xff), r.column[3]));
    return out;
}

static Mat4 mul_mat4(Mat4 l, Mat4 r) {
    Mat4 out;
    out.column[0] = linear_combine(r.column[0], l);
    out.column[1] = linear_combine(r.column[1], l);
    out.column[2] = linear_combine(r.column[2], l);
    out.column[3] = linear_combine(r.column[3], l);
    return out;
}

static Mat4 diag_mat4(f32 x) {
    Mat4 out = {0};
    out.cell[0][0] = x;
    out.cell[1][1] = x;
    out.cell[2][2] = x;
    out.cell[3][3] = x;
    return out;
}

static f32 dot_vec3(Vec3 a, Vec3 b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

static Vec3 mul_vec3_f32(Vec3 l, f32 r) {
    Vec3 out = {
        .x = l.x * r,
        .y = l.y * r,
        .z = l.z * r,
    };
    return out;
}

static Vec3 norm_vec3(Vec3 x) {
    return mul_vec3_f32(x, sqrtf(dot_vec3(x, x)));
}

static Mat4 translate_mat4(Vec3 translation) {
    Mat4 out = diag_mat4(1.0f);
    out.cell[3][0] = translation.x;
    out.cell[3][1] = translation.y;
    out.cell[3][2] = translation.z;
    return out;
}

static Mat4 scale_mat4(Vec3 scale) {
    Mat4 out = diag_mat4(1.0f);
    out.cell[0][0] = scale.x;
    out.cell[1][1] = scale.y;
    out.cell[2][2] = scale.z;
    return out;
}

static Mat4 rotate_mat4(f32 radians, Vec3 axis) {
    Mat4 out = diag_mat4(1.0f);
    Vec3 norm = norm_vec3(axis);
    f32  sin_theta = sinf(radians);
    f32  cos_theta = cosf(radians);
    f32  cos_delta = 1.0f - cos_theta;
    f32  norm_x_sin_theta = norm.x * sin_theta;
    f32  norm_y_sin_theta = norm.y * sin_theta;
    f32  norm_z_sin_theta = norm.z * sin_theta;
    f32  norm_xy_cos_delta = norm.x * norm.y * cos_delta;
    f32  norm_yz_cos_delta = norm.y * norm.z * cos_delta;
    f32  norm_xz_cos_delta = norm.x * norm.z * cos_delta;
    out.cell[0][0] = (norm.x * norm.x * cos_delta) + cos_theta;
    out.cell[0][1] = norm_xy_cos_delta + norm_z_sin_theta;
    out.cell[0][2] = norm_xz_cos_delta - norm_y_sin_theta;
    out.cell[1][0] = norm_xy_cos_delta - norm_z_sin_theta;
    out.cell[1][1] = (norm.y * norm.y * cos_delta) + cos_theta;
    out.cell[1][2] = norm_yz_cos_delta + norm_x_sin_theta;
    out.cell[2][0] = (norm.x * norm.z * cos_delta) + norm_y_sin_theta;
    out.cell[2][1] = norm_yz_cos_delta - norm_x_sin_theta;
    out.cell[2][2] = norm_xz_cos_delta + cos_theta;
    return out;
}

static Mat4 perspective_mat4(f32 fov_radians,
                             f32 aspect_ratio,
                             f32 near,
                             f32 far) {
    Mat4 out = {0};
    f32  cotangent = 1.0f / tanf(fov_radians / 2.0);
    out.cell[0][0] = cotangent / aspect_ratio;
    out.cell[1][1] = cotangent;
    out.cell[2][3] = -1.0f;
    out.cell[2][2] = (near + far) / (near - far);
    out.cell[3][2] = (2.0f * near * far) / (near - far);
    out.cell[3][3] = 0.0f;
    return out;
}

static void print_mat4(Mat4 x) {
    for (u8 i = 0; i < 4; ++i) {
        for (u8 j = 0; j < 4; ++j) {
            fprintf(stderr, " %5.2f", x.cell[i][j]);
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

#endif
