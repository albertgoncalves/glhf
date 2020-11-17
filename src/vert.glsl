#version 330 core

precision mediump float;

layout(location = 0) in vec3 IN_POS;
layout(location = 1) in vec3 IN_COLOR;
layout(location = 2) in mat4 IN_TRANSLATE;

out vec4 VERT_OUT_COLOR;

uniform float U_TIME;
uniform mat4  U_MODEL;
uniform mat4  U_VIEW;
uniform mat4  U_PROJECTION;
uniform mat4  U_TRANSFORM;

void main() {
    float t = sin(U_TIME / 5.0);
    VERT_OUT_COLOR = vec4(vec3(IN_COLOR * (t * t)), 1.0);
    // NOTE: Multiplication order matters!
    gl_Position = U_PROJECTION * U_VIEW * IN_TRANSLATE * U_TRANSFORM *
                  U_MODEL * vec4(IN_POS, 1.0);
}
