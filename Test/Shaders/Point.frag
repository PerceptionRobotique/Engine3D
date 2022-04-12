//POINT FRAGMENT
//#version 330 core
#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

varying vec4 color;
//out vec4 out_Color;

void main()
{
    gl_FragColor = vec4(color);
}
