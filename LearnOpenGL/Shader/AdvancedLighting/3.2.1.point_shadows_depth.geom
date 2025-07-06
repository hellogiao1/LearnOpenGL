//#version 330 core
//layout (triangles) in;
//layout (triangle_strip, max_vertices=18) out;
//
//uniform mat4 shadowMatrices[6];
//
//out vec4 FragPos; // FragPos from GS (output per emitvertex)
//
//void main()
//{
//    for(int face = 0; face < 6; ++face)
//    {
//        gl_Layer = face; // built-in variable that specifies to which face we render.
//        for(int i = 0; i < 3; ++i) // for each triangle's vertices
//        {
//            FragPos = gl_in[i].gl_Position;
//            gl_Position = shadowMatrices[face] * FragPos;
//            EmitVertex();
//        }
//        EndPrimitive();
//    }
//}

#version 330 core
layout (triangles) in;                // 输入：三角形图元
layout (triangle_strip, max_vertices=18) out; // 输出：18顶点（6面×3顶点）

uniform mat4 shadowMatrices[6];       // 6个面的VP矩阵
out vec4 FragPos;                     // 传递片段位置

void main() 
{
    for(int face=0; face<6; ++face) 
    {
        // 遍历立方体6个面
        gl_Layer = face;              // ★★★ 指定渲染目标面
        for(int i=0; i<3; ++i) 
        {
            // 处理三角形3个顶点
            FragPos = gl_in[i].gl_Position;
            // 应用当前面的VP变换
            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();             // 发射顶点
        }
        EndPrimitive();               // 完成当前三角形
    }
}
