#version 330 core
out vec4 FragColor;

uniform sampler2D screenTexture;

in vec2 TexCoords;

const float offset = 1.0 / 300.0;

void main()
{
    //FragColor = vec4(vec3(texture(texture_diffuse1, TexCoords)), 1.f);
    
    vec2 offsets[9] = vec2[](
    vec2(-offset,  offset), // 左上
    vec2( 0.0f,    offset), // 正上
    vec2( offset,  offset), // 右上
    vec2(-offset,  0.0f),   // 左
    vec2( 0.0f,    0.0f),   // 中
    vec2( offset,  0.0f),   // 右
    vec2(-offset, -offset), // 左下
    vec2( 0.0f,   -offset), // 正下
    vec2( offset, -offset)  // 右下
    );

    float kernel[9] = float[](
        1, 1,1,
        1, -8,1,
        1, 1,1
    );

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];

    FragColor = vec4(col, 1.0);
}