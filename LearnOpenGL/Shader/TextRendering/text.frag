#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;
uniform vec4 u_outlineColor;
uniform float u_outlineWidth = 0.03;
uniform float u_smoothness = 0.005;
uniform bool u_enableOutline = true;
uniform bool u_enableShadow = false;
uniform vec2 u_shadowOffset = vec2(2.0, 2.0);
uniform float u_shadowBlur = 0.5;

void main()
{
    // 从纹理中读取SDF距离值
    float distance = texture(text, TexCoords).r;
    
    // 计算主文字Alpha（使用smoothstep获得更好的抗锯齿效果）
    float mainAlpha = smoothstep(0.5 - u_smoothness, 0.5 + u_smoothness, distance);
    
    // 计算外轮廓Alpha
    float outlineAlpha = 0.0;
    if (u_enableOutline) {
        outlineAlpha = smoothstep(0.5 - u_outlineWidth - u_smoothness, 0.5 - u_outlineWidth + u_smoothness, distance);
    }
    
    // 计算阴影Alpha
    float shadowAlpha = 0.0;
    if (u_enableShadow) {
        vec2 shadowTexCoords = TexCoords - u_shadowOffset / vec2(1024.0, 1024.0); // 假设纹理大小为1024x1024
        if (shadowTexCoords.x >= 0.0 && shadowTexCoords.x <= 1.0 && 
            shadowTexCoords.y >= 0.0 && shadowTexCoords.y <= 1.0) {
            float shadowDistance = texture(text, shadowTexCoords).r;
            shadowAlpha = smoothstep(0.5 - u_shadowBlur, 0.5 + u_shadowBlur, shadowDistance);
        }
    }
    
    // 混合颜色：阴影 -> 轮廓 -> 文字
    vec4 finalColor = vec4(0.0, 0.0, 0.0, 0.0);
    
    // 添加阴影
    if (u_enableShadow) {
        finalColor = mix(finalColor, vec4(0.0, 0.0, 0.0, 0.5), shadowAlpha);
    }
    
    // 添加轮廓
    if (u_enableOutline) {
        finalColor = mix(finalColor, u_outlineColor, outlineAlpha);
    }
    
    // 添加主文字
    finalColor = mix(finalColor, vec4(textColor, 1.0), mainAlpha);
    
    color = finalColor;
}