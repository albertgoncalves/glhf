#version 330 core

layout(location = 0) in vec3 IN_POS;
layout(location = 1) in vec3 IN_COLOR;

out vec3 VERT_OUT_COLOR;

void main() {
    gl_Position = vec4(IN_POS, 1.0);
    VERT_OUT_COLOR = IN_COLOR;
}
