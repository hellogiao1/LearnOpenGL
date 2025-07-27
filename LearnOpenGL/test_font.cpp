#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::cout << "Testing font file existence..." << std::endl;
    
    std::string fontPath = "resources/fonts/Roboto-Bold.ttf";
    std::ifstream fontFile(fontPath);
    
    if (fontFile.good()) {
        std::cout << "✅ Font file found: " << fontPath << std::endl;
        fontFile.close();
    } else {
        std::cout << "❌ Font file not found: " << fontPath << std::endl;
        std::cout << "Please check if the font file exists in the correct location." << std::endl;
    }
    
    // 检查其他可能的字体文件
    std::string alternativeFonts[] = {
        "resources/fonts/wunderconbold.ttf",
        "LearnOpenGL/resources/fonts/Roboto-Bold.ttf",
        "LearnOpenGL/resources/fonts/wunderconbold.ttf"
    };
    
    for (const auto& altFont : alternativeFonts) {
        std::ifstream altFile(altFont);
        if (altFile.good()) {
            std::cout << "✅ Alternative font found: " << altFont << std::endl;
            altFile.close();
        }
    }
    
    std::cout << "\nFont test completed!" << std::endl;
    return 0;
} 