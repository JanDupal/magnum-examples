#version 120

attribute vec4 vertex;
attribute vec4 color;

varying vec4 varyingColor;

void main() {
    varyingColor = color;

    gl_Position = vertex;
}
