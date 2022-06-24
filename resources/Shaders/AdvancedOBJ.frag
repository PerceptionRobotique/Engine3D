//#version 330
#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

vec3 lightPos;

uniform sampler2D textureData;
uniform bool lightOnCamera;
uniform vec3 lightPosition;
uniform mat4 oMw;
uniform mat4 wMc;

uniform float opacity;

varying vec3 vertex;
varying vec3 normal;
varying vec2 texCoord;

void main()
{
    mat4 eMo = mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
    eMo[3] = vec4(-vertex, 1.0);
    vec3 lightPos;
    if(lightOnCamera)
        lightPos = vec4(eMo * oMw * wMc[3]).xyz;
    else
        lightPos = vec4(eMo * oMw * vec4(lightPosition, 1.0)).xyz;
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - gl_FragColor.xyz);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
    vec3 result =  diffuse * texture2D(textureData, texCoord).xyz;
    gl_FragColor = vec4(result, opacity);
}
