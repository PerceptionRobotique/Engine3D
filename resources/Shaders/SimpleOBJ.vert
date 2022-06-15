#version 330 core
attribute vec3 in_vertex;
in vec4 inColor;
attribute vec2 inTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 color;
out vec2 texCoord;
void main()
{
    gl_Position = projection * view * model * vec4(in_vertex, 1.0);
    color = inColor;
    texCoord = inTexCoord;
}
