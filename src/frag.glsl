#version 330 core

precision mediump float;

in vec4 VERT_OUT_COLOR;

#define K 5

void main() {
    gl_FragColor = vec4(floor(VERT_OUT_COLOR.rgb * K) / K, 1.0);
}
