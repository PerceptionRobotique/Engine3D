//VIEWER VERTEX
//#version 330 core
#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

attribute vec2 aPos;
attribute vec2 aTexCoords;

varying vec2 TexCoords;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    TexCoords = aTexCoords;
}
