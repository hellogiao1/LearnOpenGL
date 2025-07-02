#version 330 core
out vec4 FragColor;

uniform sampler2D texture_diffuse1;

in VS_OUT {
    vec2 texCoords;
} gs_in;

void main()
{
    FragColor = vec4(vec3(texture(texture_diffuse1, gs_in.texCoords)), 1.f);
}