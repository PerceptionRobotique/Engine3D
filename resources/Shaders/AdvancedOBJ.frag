#version 330

in vec2 texCoord;
in vec4 color;
in vec3 normal;
vec3 lightPos;

uniform sampler2D textureData;
out vec4 FragColor;
void main()
{
    lightPos = vec3(-1.0,-1.0, -1.0);
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - FragColor.xyz);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0,1.0,1.0);
    vec3 result =  diffuse * texture(textureData, texCoord).xyz;
    FragColor = vec4(result, 1.0);
    //FragColor = texture(textureData, texCoord);
}
