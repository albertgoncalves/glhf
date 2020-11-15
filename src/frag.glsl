#version 330 core

in vec3 VERT_OUT_COLOR;

uniform float U_TIME;

void main() {
    float t = sin(U_TIME);
    gl_FragColor = vec4(VERT_OUT_COLOR * (t * t), 1.0);
}
