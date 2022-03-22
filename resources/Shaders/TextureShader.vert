#version 330 core
attribute vec3 in_vertex;
in vec4 inColor;
attribute vec2 inTexCoord;
uniform mat4 transformation;
out vec4 color;
out vec2 texCoord;
void main()
{
    gl_Position = transformation * vec4(in_vertex, 1.0);
    color = inColor;
    texCoord = inTexCoord;
}
