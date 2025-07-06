#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
       vec3 FragPos;
       vec2 TexCoords;
       vec3 TangentLightPos;
       vec3 TangentViewPos;
       vec3 TangentFragPos;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
       vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
       vs_out.TexCoords = aTexCoords;

       mat3 normalMatrix = transpose(inverse(mat3(model)));
       vec3 T = normalize(normalMatrix * aTangent);
       vec3 N = normalize(normalMatrix * aNormal);
       // 当在更大的网格上计算切线向量的时候，它们往往有很大数量的共享顶点，当法向贴图应用到这些表面时将切线向量平均化通常能获得更好更平滑的结果。
       // 这样做有个问题，就是TBN向量可能会不能互相垂直，这意味着TBN矩阵不再是正交矩阵了。法线贴图可能会稍稍偏移，但这仍然可以改进。
       //使用叫做格拉姆-施密特正交化过程（Gram-Schmidt process）的数学技巧，我们可以对TBN向量进行重正交化，这样每个向量就又会重新垂直了
       // re-orthogonalize T with respect to N
       T = normalize(T - dot(T, N) * N);
       // then retrieve perpendicular vector B with the cross product of T and N
       vec3 B = cross(N, T);

       mat3 TBN = transpose(mat3(T, B, N));
       vs_out.TangentLightPos = TBN * lightPos;
       vs_out.TangentViewPos  = TBN * viewPos;
       vs_out.TangentFragPos  = TBN * vs_out.FragPos;

       gl_Position = projection * view * model * vec4(aPos, 1.0);
}