#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include <system_error>
#include <cctype>
#include <algorithm>

// 平台相关头文件
#if defined(_WIN32)
#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#else
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#endif

namespace FileSystem {

// 路径分隔符常量
#ifdef _WIN32
    const char PATH_SEPARATOR = '\\';
    const char ALT_PATH_SEPARATOR = '/';
#else
    const char PATH_SEPARATOR = '/';
    const char ALT_PATH_SEPARATOR = '\\';
#endif

// 规范化路径（统一分隔符）
inline std::string NormalizePath(const std::string& path) {
    std::string result = path;
    
    // 统一路径分隔符
    std::replace(result.begin(), result.end(), ALT_PATH_SEPARATOR, PATH_SEPARATOR);
    
    // 移除重复分隔符
    std::string::size_type pos;
    while ((pos = result.find(std::string(2, PATH_SEPARATOR)) != std::string::npos)) {
        result.replace(pos, 2, std::string(1, PATH_SEPARATOR));
    }
    
    // 处理相对路径中的 . 和 ..
    if (result.find("./") == 0) {
        result.erase(0, 2);
    }
    
    // 处理结尾的路径分隔符
    if (result.size() > 1 && result.back() == PATH_SEPARATOR) {
        result.pop_back();
    }
    
    return result;
}

// 组合路径
inline std::string CombinePaths(const std::string& base, const std::string& relative) {
    if (base.empty()) return relative;
    if (relative.empty()) return base;
    
    std::string normalizedBase = NormalizePath(base);
    std::string normalizedRelative = NormalizePath(relative);
    
    // 如果相对路径是绝对路径，直接返回
    #ifdef _WIN32
    if (normalizedRelative.size() > 1 && normalizedRelative[1] == ':') {
        return normalizedRelative;
    }
    #else
    if (!normalizedRelative.empty() && normalizedRelative[0] == PATH_SEPARATOR) {
        return normalizedRelative;
    }
    #endif
    
    // 组合路径
    if (normalizedBase.back() == PATH_SEPARATOR) {
        return normalizedBase + normalizedRelative;
    }
    return normalizedBase + PATH_SEPARATOR + normalizedRelative;
}

// 检查文件是否存在
inline bool FileExists(const std::string& path) {
    #ifdef _WIN32
        DWORD attrib = GetFileAttributesA(path.c_str());
        return (attrib != INVALID_FILE_ATTRIBUTES && 
                !(attrib & FILE_ATTRIBUTE_DIRECTORY));
    #else
        struct stat buffer;
        if (stat(path.c_str(), &buffer) return false;
        return S_ISREG(buffer.st_mode);
    #endif
}

// 检查路径是否存在（文件或目录）
inline bool PathExists(const std::string& path) {
    #ifdef _WIN32
        DWORD attrib = GetFileAttributesA(path.c_str());
        return (attrib != INVALID_FILE_ATTRIBUTES);
    #else
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0);
    #endif
}

// 获取当前工作目录
inline std::string GetCurrentDirectory() {
    #ifdef _WIN32
        char buffer[MAX_PATH];
        DWORD len = GetCurrentDirectoryA(MAX_PATH, buffer);
        if (len == 0 || len > MAX_PATH) return "";
        return std::string(buffer, len);
    #else
        char buffer[PATH_MAX];
        if (getcwd(buffer, PATH_MAX) == nullptr) return "";
        return std::string(buffer);
    #endif
}

// 获取绝对路径
inline std::string GetAbsolutePath(const std::string& path) {
    #ifdef _WIN32
        char fullPath[MAX_PATH];
        DWORD len = GetFullPathNameA(path.c_str(), MAX_PATH, fullPath, nullptr);
        if (len == 0 || len > MAX_PATH) return "";
        return std::string(fullPath, len);
    #else
        char buffer[PATH_MAX];
        if (realpath(path.c_str(), buffer) == nullptr) return "";
        return std::string(buffer);
    #endif
}

// 获取目录路径
inline std::string GetDirectoryPath(const std::string& path) {
    size_t pos = path.find_last_of(PATH_SEPARATOR);
    if (pos == std::string::npos) return "";
    return path.substr(0, pos);
}

// 获取文件名（带扩展名）
inline std::string GetFileName(const std::string& path) {
    size_t pos = path.find_last_of(PATH_SEPARATOR);
    if (pos == std::string::npos) return path;
    return path.substr(pos + 1);
}

// 获取文件扩展名
inline std::string GetFileExtension(const std::string& path) {
    std::string filename = GetFileName(path);
    size_t pos = filename.find_last_of('.');
    if (pos == std::string::npos) return "";
    return filename.substr(pos + 1);
}

// 获取不带扩展名的文件名
inline std::string GetFileNameWithoutExtension(const std::string& path) {
    std::string filename = GetFileName(path);
    size_t pos = filename.find_last_of('.');
    if (pos == std::string::npos) return filename;
    return filename.substr(0, pos);
}

// 创建目录（包括父目录）
inline bool CreateDirectoryRecursive(const std::string& path) {
    std::string normalized = NormalizePath(path);
    
    #ifdef _WIN32
        if (PathExists(normalized)) return true;
        
        // 创建所有父目录
        for (size_t pos = 0; (pos = normalized.find(PATH_SEPARATOR, pos + 1)) != std::string::npos; ) {
            std::string part = normalized.substr(0, pos);
            if (!PathExists(part) && !CreateDirectoryA(part.c_str(), nullptr)) {
                return false;
            }
        }
        
        return CreateDirectoryA(normalized.c_str(), nullptr) != 0;
    #else
        if (PathExists(normalized)) return true;
        
        std::string current;
        size_t pos = 0;
        while ((pos = normalized.find(PATH_SEPARATOR, pos)) != std::string::npos) {
            current = normalized.substr(0, pos++);
            if (current.empty()) continue; // 跳过根目录
            
            if (mkdir(current.c_str(), 0755) && errno != EEXIST) {
                return false;
            }
        }
        
        return mkdir(normalized.c_str(), 0755) == 0 || errno == EEXIST;
    #endif
}

} // namespace FileSystem

// ======================
// 测试函数
// ======================

inline void TestPathCombination(const std::string& base, const std::string& relative) {
    std::string combined = FileSystem::CombinePaths(base, relative);
    std::string absolute = FileSystem::GetAbsolutePath(combined);
    
    std::cout << "组合路径测试:\n";
    std::cout << "  基础路径: " << base << "\n";
    std::cout << "  相对路径: " << relative << "\n";
    std::cout << "  组合结果: " << combined << "\n";
    std::cout << "  绝对路径: " << absolute << "\n";
    
    bool exists = FileSystem::PathExists(combined);
    bool isFile = FileSystem::FileExists(combined);
    
    std::cout << "  路径存在: " << (exists ? "是" : "否") << "\n";
    std::cout << "  是文件: " << (isFile ? "是" : "否") << "\n";
    
    if (exists && !isFile) {
        std::cout << "  目录内容: ";
        // 这里可以添加目录列表功能
    }
    
    std::cout << "\n";
}

inline void PrintPathInfo(const std::string& path) {
    std::cout << "路径分析:\n";
    std::cout << "  原始路径: " << path << "\n";
    std::cout << "  规范化: " << FileSystem::NormalizePath(path) << "\n";
    std::cout << "  目录路径: " << FileSystem::GetDirectoryPath(path) << "\n";
    std::cout << "  文件名: " << FileSystem::GetFileName(path) << "\n";
    std::cout << "  文件名(无扩展名): " << FileSystem::GetFileNameWithoutExtension(path) << "\n";
    std::cout << "  扩展名: " << FileSystem::GetFileExtension(path) << "\n";
    std::cout << "\n";
}

/*int main() {
    std::cout << "当前工作目录: " << FileSystem::GetCurrentDirectory() << "\n\n";
    
    // 测试路径组合
    TestPathCombination("C:\\Projects", "MyApp\\src\\main.cpp");
    TestPathCombination("/home/user", "documents/report.txt");
    TestPathCombination("data", "config/settings.ini");
    
    // 测试绝对路径
    std::string absPath = FileSystem::GetAbsolutePath("test.txt");
    std::cout << "测试文件绝对路径: " << absPath << "\n";
    std::cout << "文件存在: " << (FileSystem::FileExists(absPath) ? "是" : "否") << "\n\n";
    
    // 测试路径分析
    PrintPathInfo("C:\\Projects\\MyApp\\src\\main.cpp");
    PrintPathInfo("/home/user/documents/report.pdf");
    PrintPathInfo("data/images/logo.png");
    
    // 测试目录创建
    std::string newDir = "new_directory/sub1/sub2";
    std::cout << "创建目录: " << newDir << "\n";
    if (FileSystem::CreateDirectoryRecursive(newDir)) {
        std::cout << "创建成功!\n";
    } else {
        std::cout << "创建失败!\n";
    }
    
    return 0;
}*/