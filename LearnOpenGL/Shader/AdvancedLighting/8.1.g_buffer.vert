#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
       vec4 worldPos = model * vec4(aPos, 1.0);
       FragPos = worldPos.xyz;
       TexCoords = aTexCoords;

      /** mat3 normalMatrix = transpose(inverse(mat3(model)));
       Normal = normalMatrix * aNormal;*/

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

       TBN = mat3(T, B, N);

       gl_Position = projection * view * worldPos;
}