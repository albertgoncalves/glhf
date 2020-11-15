#version 330 core

layout(location = 0) in vec3 IN_POS;
layout(location = 1) in vec3 IN_COLOR;

out vec3 VERT_OUT_COLOR;

uniform mat4 U_TRANSFORM;

void main() {
    gl_Position = U_TRANSFORM * vec4(IN_POS, 1.0);
    VERT_OUT_COLOR = IN_COLOR;
}
