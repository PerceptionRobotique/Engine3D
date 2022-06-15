#version 330 core
attribute vec3 in_vertex;
in vec4 inColor;
attribute vec3 in_normal;
attribute vec2 inTexCoord;

uniform mat4 modelview;
uniform mat4 projection;
out vec4 color;
out vec3 normal;
out vec2 texCoord;
void main()
{
    gl_Position = projection * modelview * vec4(in_vertex, 1.0);

    color = inColor;
    normal = in_normal;
    texCoord = inTexCoord;
}
