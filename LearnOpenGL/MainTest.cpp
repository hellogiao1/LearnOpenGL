#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/vec1.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "shader_s.h"
#include "stb_image.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color);
void RenderSDFText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

/// SDF Character structure
struct SDFCharacter {
    GLuint textureID;
    glm::ivec2 size;
    glm::ivec2 bearing;
    FT_Pos advance;
    glm::vec2 atlasPos; // 在图集中的位置
    glm::vec2 atlasSize; // 在图集中的尺寸
};

std::map<GLchar, Character> Characters;
std::map<GLchar, SDFCharacter> SDFCharacters;
unsigned int VAO, VBO;

// SDF相关变量
GLuint sdfAtlasTexture = 0;
bool useSDF = true; // 切换开关
bool enableOutline = true;
bool enableShadow = false;
float outlineWidth = 0.03f;  // 进一步减小轮廓宽度
float smoothness = 0.005f;   // 进一步减小平滑度，让文字更清晰

// 生成SDF纹理的函数
unsigned char* GenerateSDFTexture(const unsigned char* bitmap, int width, int height, int spread) {
    int sdfWidth = width + 2 * spread;
    int sdfHeight = height + 2 * spread;
    unsigned char* sdfData = new unsigned char[sdfWidth * sdfHeight];
    
    // 初始化SDF数据
    for (int i = 0; i < sdfWidth * sdfHeight; i++) {
        sdfData[i] = 0;
    }
    
    // 计算有向距离场
    for (int y = 0; y < sdfHeight; y++) {
        for (int x = 0; x < sdfWidth; x++) {
            float minDist = spread;
            
            // 检查原始位图中的每个像素
            for (int sy = 0; sy < height; sy++) {
                for (int sx = 0; sx < width; sx++) {
                    if (bitmap[sy * width + sx] > 0) { // 如果原始像素是白色
                        float dist = sqrt((x - sx - spread) * (x - sx - spread) + 
                                        (y - sy - spread) * (y - sy - spread));
                        if (dist < minDist) {
                            minDist = dist;
                        }
                    }
                }
            }
            
            // 归一化到0-255范围，使用更精确的SDF计算
            float normalizedDist = 1.0f - minDist / spread;
            sdfData[y * sdfWidth + x] = (unsigned char)(255.0f * normalizedDist);
        }
    }
    
    return sdfData;
}

// 加载SDF字体
void LoadSDFFont(const std::string& fontPath, int fontSize, int spread = 8) {
    std::cout << "Loading SDF font: " << fontPath << std::endl;
    
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }
    
    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font: " << fontPath << std::endl;
        return;
    }
    
    std::cout << "Font loaded successfully. Setting pixel size: " << fontSize << std::endl;
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    
    // 计算图集大小
    int atlasWidth = 1024;
    int atlasHeight = 1024;
    int currentX = 0;
    int currentY = 0;
    int maxHeight = 0;
    
    // 创建图集纹理
    unsigned char* atlasData = new unsigned char[atlasWidth * atlasHeight];
    memset(atlasData, 0, atlasWidth * atlasHeight);
    
    // 为每个字符生成SDF纹理并添加到图集
    for (GLubyte c = 32; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYPE: Failed to load Glyph for character: " << (char)c << std::endl;
            continue;
        }
        
        FT_GlyphSlot glyph = face->glyph; // 获取当前字符的位图数据
        int width = glyph->bitmap.width;
        int height = glyph->bitmap.rows;
        
        if (width == 0 || height == 0) {
            SDFCharacter character;
            character.textureID = 0;
            character.size = glm::ivec2(0, 0);
            character.bearing = glm::ivec2(glyph->bitmap_left, glyph->bitmap_top);
            character.advance = glyph->advance.x;
            character.atlasPos = glm::vec2(0.0f, 0.0f);
            character.atlasSize = glm::vec2(0.0f, 0.0f);
            SDFCharacters[c] = character;
            continue;
        }
        
        std::cout << "Processing character '" << (char)c << "' size: " << width << "x" << height << std::endl;
        
        // 生成SDF纹理
        unsigned char* sdfData = GenerateSDFTexture(glyph->bitmap.buffer, width, height, spread);
        int sdfWidth = width + 2 * spread;
        int sdfHeight = height + 2 * spread;
        
        // 检查是否需要换行
        if (currentX + sdfWidth > atlasWidth) {
            currentX = 0;
            currentY += maxHeight;
            maxHeight = 0;
        }
        
        // 检查是否需要更大的图集
        if (currentY + sdfHeight > atlasHeight) {
            std::cout << "ERROR: Atlas too small for all characters" << std::endl;
            delete[] sdfData;
            break;
        }
        
        // 将SDF数据复制到图集
        for (int y = 0; y < sdfHeight; y++) {
            for (int x = 0; x < sdfWidth; x++) {
                int atlasIndex = (currentY + y) * atlasWidth + (currentX + x);
                int sdfIndex = y * sdfWidth + x;
                atlasData[atlasIndex] = sdfData[sdfIndex];
            }
        }
        
        // 存储字符信息
        SDFCharacter character;
        character.textureID = 0; // 纹理ID将在后面设置
        character.size = glm::ivec2(sdfWidth, sdfHeight);
        character.bearing = glm::ivec2(glyph->bitmap_left, glyph->bitmap_top);
        character.advance = glyph->advance.x;
        character.atlasPos.x = (float)currentX / atlasWidth;
        character.atlasPos.y = (float)currentY / atlasHeight;
        character.atlasSize.x = (float)sdfWidth / atlasWidth;
        character.atlasSize.y = (float)sdfHeight / atlasHeight;
        
        SDFCharacters[c] = character;
        
        currentX += sdfWidth;
        if (sdfHeight > maxHeight) {
            maxHeight = sdfHeight;
        }
        
        delete[] sdfData;
    }
    
    std::cout << "Created " << SDFCharacters.size() << " SDF characters" << std::endl;
    
    // 创建OpenGL纹理
    glGenTextures(1, &sdfAtlasTexture);
    glBindTexture(GL_TEXTURE_2D, sdfAtlasTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, atlasData);
    
    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // 更新所有字符的纹理ID
    for (auto& char_pair : SDFCharacters) {
        char_pair.second.textureID = sdfAtlasTexture;
    }
    
    delete[] atlasData;
    
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
    std::cout << "SDF font loading completed successfully!" << std::endl;
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL - SDF Text Rendering", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // OpenGL state
    // ------------
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // compile and setup the shader
    // ----------------------------
    Shader shader("TextRendering/text.vs", "TextRendering/text.fs");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // 加载SDF字体
    LoadSDFFont("resources/fonts/Roboto-Bold.ttf", 48, 6);  // 优化spread参数

    // configure VAO/VBO for texture quads
    // -----------------------------------
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (useSDF) {
            // 使用SDF渲染
            RenderSDFText(shader, "SDF Text Rendering", 25.0f, 500.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
            RenderSDFText(shader, "High Quality Text", 25.0f, 450.0f, 0.8f, glm::vec3(1.0, 0.5f, 0.2f));
            RenderSDFText(shader, "Scalable Fonts", 25.0f, 400.0f, 0.6f, glm::vec3(0.3, 0.7f, 0.9f));
            RenderSDFText(shader, "Perfect Anti-aliasing", 25.0f, 350.0f, 0.4f, glm::vec3(0.8f, 0.2f, 0.8f));
        } else {
            // 使用传统渲染
            RenderText(shader, "Traditional Text Rendering", 25.0f, 500.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
        }
        
        // 显示控制提示
        RenderSDFText(shader, "SPACE: Toggle SDF/Traditional", 25.0f, 100.0f, 0.4f, glm::vec3(1.0, 1.0f, 1.0f));
        RenderSDFText(shader, "O: Toggle Outline", 25.0f, 70.0f, 0.4f, glm::vec3(1.0, 1.0f, 1.0f));
        RenderSDFText(shader, "S: Toggle Shadow", 25.0f, 40.0f, 0.4f, glm::vec3(1.0, 1.0f, 1.0f));
        
        // 显示当前状态
        std::string modeText = useSDF ? "SDF Mode" : "Traditional Mode";
        RenderSDFText(shader, modeText, 500.0f, 500.0f, 0.5f, glm::vec3(1.0, 0.8f, 0.2f));
       
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // 按空格键切换渲染模式
    static bool spacePressed = false;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed) {
        useSDF = !useSDF;
        spacePressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        spacePressed = false;
    }
    
    // 按O键切换轮廓
    static bool oPressed = false;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && !oPressed) {
        enableOutline = !enableOutline;
        oPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE) {
        oPressed = false;
    }
    
    // 按S键切换阴影
    static bool sPressed = false;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !sPressed) {
        enableShadow = !enableShadow;
        sPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE) {
        sPressed = false;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// render line of text (traditional method)
// -------------------
void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state	
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// render line of text (SDF method)
// -------------------
void RenderSDFText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
    // 检查SDF字符是否已加载
    if (SDFCharacters.empty()) {
        std::cout << "WARNING: No SDF characters loaded! Trying to load font again..." << std::endl;
        LoadSDFFont("resources/fonts/Roboto-Bold.ttf", 48, 6);  // 优化spread参数
        if (SDFCharacters.empty()) {
            std::cout << "ERROR: Still no SDF characters loaded!" << std::endl;
            return;
        }
    }
    
    // activate corresponding render state	
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glUniform4f(glGetUniformLocation(shader.ID, "u_outlineColor"), 0.0f, 0.0f, 0.0f, 1.0f);
    glUniform1f(glGetUniformLocation(shader.ID, "u_outlineWidth"), outlineWidth);
    glUniform1f(glGetUniformLocation(shader.ID, "u_smoothness"), smoothness);
    glUniform1i(glGetUniformLocation(shader.ID, "u_enableOutline"), enableOutline);
    glUniform1i(glGetUniformLocation(shader.ID, "u_enableShadow"), enableShadow);
    glUniform2f(glGetUniformLocation(shader.ID, "u_shadowOffset"), 2.0f, 2.0f);
    glUniform1f(glGetUniformLocation(shader.ID, "u_shadowBlur"), 0.5f);
    glUniform1i(glGetUniformLocation(shader.ID, "text"), 0); // 显式设置采样器uniform
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sdfAtlasTexture);
    glBindVertexArray(VAO);

    // iterate through all characters
    std::string::const_iterator c;
    int renderedChars = 0;
    for (c = text.begin(); c != text.end(); c++) 
    {
        if (SDFCharacters.find(*c) == SDFCharacters.end()) {
            std::cout << "WARNING: Character '" << *c << "' not found in SDF characters!" << std::endl;
            continue;
        }
        
        SDFCharacter ch = SDFCharacters[*c];
        renderedChars++;

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;  // 确保字符垂直对齐

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        
        // 计算UV坐标
        float u1 = ch.atlasPos.x;
        float v1 = ch.atlasPos.y;
        float u2 = ch.atlasPos.x + ch.atlasSize.x;
        float v2 = ch.atlasPos.y + ch.atlasSize.y;
        
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   u1, v1 },            
            { xpos,     ypos,       u1, v2 },
            { xpos + w, ypos,       u2, v2 },

            { xpos,     ypos + h,   u1, v1 },
            { xpos + w, ypos,       u2, v2 },
            { xpos + w, ypos + h,   u2, v1 }           
        };
        
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (增加字符间距)
        x += ((ch.advance >> 6) + 2) * scale;  // 增加2像素的额外间距
    }
    
    if (renderedChars == 0) {
        std::cout << "WARNING: No characters rendered for text: " << text << std::endl;
    } else {
        std::cout << "Rendered " << renderedChars << " characters for text: " << text << std::endl;
    }
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}