#version 330 core

layout(location = 0) in vec3 IN_POS;
layout(location = 1) in vec3 IN_COLOR;

out vec3 VERT_OUT_COLOR;

uniform mat4 U_TRANSFORM;
uniform mat4 U_MODEL;
uniform mat4 U_VIEW;
uniform mat4 U_PROJECTION;

void main() {
    gl_Position =
        U_PROJECTION * U_VIEW * U_MODEL * U_TRANSFORM * vec4(IN_POS, 1.0);
    VERT_OUT_COLOR = IN_COLOR;
}
