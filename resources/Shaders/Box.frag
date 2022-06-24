//POINT FRAGMENT
//#version 330 core
#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

uniform vec3 color;
uniform float opacity;

void main()
{
    gl_FragColor = vec4(color, opacity);
}
