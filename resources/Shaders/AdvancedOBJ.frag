//#version 330
#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

uniform bool hasTexture;
uniform vec3 faceColor;
uniform sampler2D textureData;
uniform bool lightOnCamera;
uniform vec3 lightPosition;
uniform mat4 wMo;
uniform mat4 oMw;
uniform mat4 wMc;
uniform mat4 cMw;
uniform mat4 iMc;

uniform float opacity;
uniform float globalIllumination;

varying vec3 vertex;
varying vec3 normal;
varying vec2 texCoord;

void main()
{
    vec4 viewPosition = (cMw * wMo * vec4(vertex, 1.0));
    vec3 fragPosition = viewPosition.xyz / viewPosition.w;

    vec3 lightPos;
    if(lightOnCamera) lightPos = wMc[3].xyz;
    else lightPos = lightPosition;

    // Direction de la lumière par rapport au fragment
    vec3 lightDir = normalize(lightPos - fragPosition);

    vec4 objectColor = texture2D(textureData, texCoord);

    // Calcul de l'intensité diffuse
    float diffuse = max(dot(normal, lightDir), dot(-normal, lightDir));

    // Calcul de la couleur diffuse
    vec3 diffuseColor = objectColor.xyz * diffuse;

    // Calcul de la lumière ambiante
    vec3 ambientColor = objectColor.xyz * globalIllumination;

    // Couleur finale
    vec3 finalColor = diffuseColor + ambientColor;

    // Sortie de la couleur du fragment
    gl_FragColor = vec4(finalColor, opacity);
}
