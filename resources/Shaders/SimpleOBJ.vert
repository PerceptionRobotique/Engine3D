//#version 330 core
#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

attribute vec3 in_vertex;
//in vec4 in_color;
attribute vec2 in_uv;

uniform mat4 wMo;
uniform mat4 cMw;
uniform mat4 iMc;

out vec4 color;
out vec2 texCoord;
void main()
{
    gl_Position = iMc * cMw * wMo * vec4(in_vertex, 1.0);
    //color = in_color;
    texCoord = in_uv;
}
