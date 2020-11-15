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

typedef __m128 Simd4_f32;

typedef union {
    f32       cell[4][4];
    Simd4_f32 column[4];
} Mat4;

static Simd4_f32 linear_combine(Simd4_f32 l, Mat4 r) {
    Simd4_f32 out;
    out = _mm_mul_ps(_mm_shuffle_ps(l, l, 0x00), r.column[0]);
    out = _mm_add_ps(out, _mm_mul_ps(_mm_shuffle_ps(l, l, 0x55), r.column[1]));
    out = _mm_add_ps(out, _mm_mul_ps(_mm_shuffle_ps(l, l, 0xaa), r.column[2]));
    out = _mm_add_ps(out, _mm_mul_ps(_mm_shuffle_ps(l, l, 0xff), r.column[3]));
    return out;
}

static Mat4 mul_mat4_mat4(Mat4 l, Mat4 r) {
    Mat4 out;
    out.column[0] = linear_combine(r.column[0], l);
    out.column[1] = linear_combine(r.column[1], l);
    out.column[2] = linear_combine(r.column[2], l);
    out.column[3] = linear_combine(r.column[3], l);
    return out;
}

static Mat4 mat4_diag(f32 x) {
    Mat4 out = {0};
    out.cell[0][0] = x;
    out.cell[1][1] = x;
    out.cell[2][2] = x;
    out.cell[3][3] = x;
    return out;
}

static f32 vec3_dot(Vec3 a, Vec3 b) {
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

static Vec3 vec3_norm(Vec3 x) {
    return mul_vec3_f32(x, sqrtf(vec3_dot(x, x)));
}

static Mat4 translate(Vec3 translation) {
    Mat4 out = mat4_diag(1.0f);
    out.cell[3][0] = translation.x;
    out.cell[3][1] = translation.y;
    out.cell[3][2] = translation.z;
    return out;
}

static Mat4 scale(Vec3 scale) {
    Mat4 out = mat4_diag(1.0f);
    out.cell[0][0] = scale.x;
    out.cell[1][1] = scale.y;
    out.cell[2][2] = scale.z;
    return out;
}

static Mat4 rotate(f32 radians, Vec3 axis) {
    Mat4 out = mat4_diag(1.0f);
    Vec3 norm = vec3_norm(axis);
    f32  sin_theta = sinf(radians);
    f32  cos_theta = cosf(radians);
    f32  cos_delta = 1.0f - cos_theta;
    out.cell[0][0] = (norm.x * norm.x * cos_delta) + cos_theta;
    out.cell[0][1] = (norm.x * norm.y * cos_delta) + (norm.z * sin_theta);
    out.cell[0][2] = (norm.x * norm.z * cos_delta) - (norm.y * sin_theta);
    out.cell[1][0] = (norm.y * norm.x * cos_delta) - (norm.z * sin_theta);
    out.cell[1][1] = (norm.y * norm.y * cos_delta) + cos_theta;
    out.cell[1][2] = (norm.y * norm.z * cos_delta) + (norm.x * sin_theta);
    out.cell[2][0] = (norm.z * norm.x * cos_delta) + (norm.y * sin_theta);
    out.cell[2][1] = (norm.z * norm.y * cos_delta) - (norm.x * sin_theta);
    out.cell[2][2] = (norm.z * norm.z * cos_delta) + cos_theta;
    return out;
}

#endif
