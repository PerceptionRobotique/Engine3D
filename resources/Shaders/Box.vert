//POINT VERTEX
//#version 330 core
#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

uniform mat4 wMo;
uniform mat4 cMw;
uniform mat4 iMc;

attribute vec3 pos;
uniform vec3 in_color;
uniform float opacity;

varying vec4 color;

void main()
{
    gl_Position = iMc * cMw * wMo * vec4(pos, 1.0);
    color = vec4(in_color, opacity);
}
