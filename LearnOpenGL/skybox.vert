#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

/**uniform mat4 projection;
uniform mat4 view;*/

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

uniform mat4 SkyView;

void main()
{
   /** TexCoords = aPos;
    gl_Position = projection * view * vec4(aPos, 1.0);*/

    // 提前深度测试
    TexCoords = aPos;
    vec4 pos = projection * SkyView * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}