//POINT VERTEX
//#version 330 core
#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

uniform mat4 wMo;
uniform mat4 cMw;
uniform mat4 iMc;

attribute vec3 in_vertex;

void main()
{
    gl_Position = iMc * cMw * wMo * vec4(in_vertex, 1.0);
}
