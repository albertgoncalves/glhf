#version 330 core

in vec4 VERT_OUT_COLOR;

void main() {
    gl_FragColor = VERT_OUT_COLOR;
}
