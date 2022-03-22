#version 330 core
in vec4 color;
out vec4 out_Color;

void main()
{
    out_Color = vec4(color.r, color.g,color.b, color.a);
}
