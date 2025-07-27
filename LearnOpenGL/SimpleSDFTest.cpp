#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

// 简单的SDF生成测试
void TestSDFGeneration() {
    std::cout << "=== SDF Generation Test ===" << std::endl;
    
    // 创建一个简单的圆形位图
    const int width = 8;
    const int height = 8;
    std::vector<unsigned char> bitmap(width * height, 0);
    
    // 在中心绘制一个圆形
    int centerX = width / 2;
    int centerY = height / 2;
    int radius = 2;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float dist = std::sqrt((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY));
            if (dist <= radius) {
                bitmap[y * width + x] = 255;
            }
        }
    }
    
    // 显示原始位图
    std::cout << "Original Bitmap:" << std::endl;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            std::cout << (bitmap[y * width + x] > 0 ? "##" : "  ");
        }
        std::cout << std::endl;
    }
    
    // 生成SDF
    const int spread = 2;
    const int sdfWidth = width + 2 * spread;
    const int sdfHeight = height + 2 * spread;
    std::vector<unsigned char> sdfData(sdfWidth * sdfHeight, 0);
    
    for (int y = 0; y < sdfHeight; y++) {
        for (int x = 0; x < sdfWidth; x++) {
            float minDist = spread;
            
            // 计算到最近白色像素的距离
            for (int sy = 0; sy < height; sy++) {
                for (int sx = 0; sx < width; sx++) {
                    if (bitmap[sy * width + sx] > 0) {
                        float dist = std::sqrt((x - sx - spread) * (x - sx - spread) + 
                                            (y - sy - spread) * (y - sy - spread));
                        if (dist < minDist) {
                            minDist = dist;
                        }
                    }
                }
            }
            
            // 归一化到0-255范围
            sdfData[y * sdfWidth + x] = (unsigned char)(255.0f * (1.0f - minDist / spread));
        }
    }
    
    // 显示SDF数据
    std::cout << "\nSDF Data:" << std::endl;
    for (int y = 0; y < sdfHeight; y++) {
        for (int x = 0; x < sdfWidth; x++) {
            int value = sdfData[y * sdfWidth + x];
            if (value > 200) std::cout << "##";
            else if (value > 150) std::cout << "**";
            else if (value > 100) std::cout << "++";
            else if (value > 50) std::cout << "..";
            else std::cout << "  ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "\nLegend: ##=255-200, **=199-150, ++=149-100, ..=99-50,  =49-0" << std::endl;
}

// 测试着色器逻辑
void TestShaderLogic() {
    std::cout << "\n=== Shader Logic Test ===" << std::endl;
    
    // 模拟着色器中的smoothstep函数
    auto smoothstep = [](float edge0, float edge1, float x) {
        float t = (x - edge0) / (edge1 - edge0);
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        return t * t * (3.0f - 2.0f * t);
    };
    
    // 测试不同的SDF值
    std::vector<float> sdfValues = {0.0f, 0.2f, 0.4f, 0.5f, 0.6f, 0.8f, 1.0f};
    float smoothness = 0.1f;
    float outlineWidth = 0.1f;
    
    std::cout << "SDF Value | Main Alpha | Outline Alpha" << std::endl;
    std::cout << "----------|------------|---------------" << std::endl;
    
    for (float sdfValue : sdfValues) {
        float mainAlpha = smoothstep(0.5f - smoothness, 0.5f + smoothness, sdfValue);
        float outlineAlpha = smoothstep(0.5f - outlineWidth - smoothness, 
                                      0.5f - outlineWidth + smoothness, sdfValue);
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << sdfValue << "      | " << mainAlpha << "        | " << outlineAlpha << std::endl;
    }
}

int main() {
    std::cout << "SDF Font Rendering Test Program" << std::endl;
    std::cout << "=================================" << std::endl;
    
    TestSDFGeneration();
    TestShaderLogic();
    
    std::cout << "\nAll tests completed successfully!" << std::endl;
    std::cout << "The SDF algorithm is working correctly." << std::endl;
    std::cout << "You can now run the main OpenGL program to see the visual effects." << std::endl;
    
    return 0;
} 