//VIEWER FRAGMENT
//#version 330 core
#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

//out vec4 FragColor;

varying vec2 TexCoords;

uniform float IdRatio;
uniform sampler2D screenTexture;
uniform sampler2D IdTexture;

void main()
{
    gl_FragColor = mix(texture2D(screenTexture, TexCoords), texture2D(IdTexture, TexCoords), IdRatio);
}
