#version 330 core
attribute vec3 in_vertex;
uniform float R;
uniform float G;
uniform float B;

uniform mat4 transformation;

out vec4 color;

void main()
{
    gl_Position = transformation * vec4(in_vertex, 1.0);
    color = vec4(R,G,B, 1.0);
}
