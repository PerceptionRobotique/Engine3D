#version 330

in vec2 texCoord;
in vec4 color;
uniform sampler2D textureData;
out vec4 FragColor;
void main()
{
    FragColor = texture(textureData, texCoord);
}
