#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <mutex>
#include <iomanip>  // 添加用于时间格式化的头文件
#include <sstream>

// ======================
// 日志系统实现 (修复版)
// ======================

enum class LogLevel {
    Trace,   // 最详细的跟踪信息
    Debug,   // 调试信息
    Info,    // 常规信息
    Warning, // 警告信息
    Error,   // 错误信息
    Fatal    // 严重错误
};

class Logger {
public:
    static Logger& get() {
        static Logger instance;
        return instance;
    }

    void setLogFile(const std::string& filename) {
        std::lock_guard<std::mutex> lock(logMutex);
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "无法打开日志文件: " << filename << std::endl;
        }
    }

    void setMinLevel(LogLevel level) {
        minLevel = level;
    }

    void log(LogLevel level, const std::string& message, 
             const char* file = "", int line = 0) {
        if (level < minLevel) return;

        std::lock_guard<std::mutex> lock(logMutex);
        
        // 获取当前时间 - 使用安全的格式化方法
        auto now = std::time(nullptr);
        std::tm tm;
#if defined(_WIN32)
        localtime_s(&tm, &now);  // Windows 安全版本
#else
        localtime_r(&now, &tm);  // POSIX 安全版本
#endif

        // 格式化时间字符串
        std::ostringstream timeStream;
        timeStream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        std::string timeStr = timeStream.str();

        // 日志级别字符串
        std::string levelStr;
        switch (level) {
            case LogLevel::Trace:   levelStr = "TRACE"; break;
            case LogLevel::Debug:   levelStr = "DEBUG"; break;
            case LogLevel::Info:    levelStr = "INFO "; break;
            case LogLevel::Warning: levelStr = "WARN "; break;
            case LogLevel::Error:   levelStr = "ERROR"; break;
            case LogLevel::Fatal:   levelStr = "FATAL"; break;
        }

        // 构建日志消息
        std::ostringstream oss;
        oss << "[" << timeStr << "] "
            << "[" << levelStr << "] ";
            
        if (file && *file) {
            // 只显示文件名，不显示完整路径
            const char* filename = file;
            if (const char* pos = std::strrchr(file, '/')) {
                filename = pos + 1;
            } else if (const char* pos = std::strrchr(file, '\\')) {
                filename = pos + 1;
            }
            oss << "[" << filename << ":" << line << "] ";
        }
        
        oss << message;

        std::string logMessage = oss.str();

        // 输出到控制台
        if (level >= LogLevel::Warning) {
            std::cerr << logMessage << std::endl;
        } else {
            std::cout << logMessage << std::endl;
        }

        // 输出到文件
        if (logFile.is_open()) {
            logFile << logMessage << '\n';  // 使用 '\n' 代替 std::endl 避免频繁刷新
            logFile.flush();  // 确保及时写入
        }

        // 严重错误处理
        if (level == LogLevel::Fatal) {
            std::cerr << "\n!!! FATAL ERROR !!! 程序将终止" << std::endl;
            std::abort();
        }
    }

private:
    Logger() : minLevel(LogLevel::Info) {}
    ~Logger() {
        std::lock_guard<std::mutex> lock(logMutex);
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    std::ofstream logFile;
    LogLevel minLevel;
    std::mutex logMutex;
};

// 日志宏 (带文件名和行号)
#define LOG_TRACE(msg)   Logger::get().log(LogLevel::Trace, (msg), __FILE__, __LINE__)
#define LOG_DEBUG(msg)   Logger::get().log(LogLevel::Debug, (msg), __FILE__, __LINE__)
#define LOG_INFO(msg)    Logger::get().log(LogLevel::Info, (msg), __FILE__, __LINE__)
#define LOG_WARN(msg)    Logger::get().log(LogLevel::Warning, (msg), __FILE__, __LINE__)
#define LOG_ERROR(msg)   Logger::get().log(LogLevel::Error, (msg), __FILE__, __LINE__)
#define LOG_FATAL(msg)   Logger::get().log(LogLevel::Fatal, (msg), __FILE__, __LINE__)