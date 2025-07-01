// ======================
// 断言系统实现 (修复版)
// ======================

// 触发断点函数
#if defined(_MSC_VER)
    #define DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
    #define DEBUG_BREAK() __builtin_trap()
#else
    #include <cstdlib>
    #define DEBUG_BREAK() (std::abort())
#endif

// 断言宏
#define CHECK(condition) \
    do { \
        if (!(condition)) { \
            std::ostringstream oss; \
            oss << "\n!!! ASSERTION FAILED !!!\n" \
                << "Expression: " << #condition << "\n" \
                << "File: " << __FILE__ << "\n" \
                << "Line: " << __LINE__; \
            LOG_ERROR(oss.str()); \
            DEBUG_BREAK(); \
        } \
    } while(0)

// 带消息的断言宏
#define CHECK_MSG(condition, msg) \
    do { \
        if (!(condition)) { \
            std::ostringstream oss; \
            oss << "\n!!! ASSERTION FAILED !!!\n" \
                << "Expression: " << #condition << "\n" \
                << "Message: " << (msg) << "\n" \
                << "File: " << __FILE__ << "\n" \
                << "Line: " << __LINE__; \
            LOG_ERROR(oss.str()); \
            DEBUG_BREAK(); \
        } \
    } while(0)

// 检查指针有效性
#define CHECK_PTR(ptr) \
    do { \
        if (!(ptr)) { \
            std::ostringstream oss; \
            oss << "\n!!! NULL POINTER CHECK FAILED !!!\n" \
                << "Pointer: " << #ptr << "\n" \
                << "File: " << __FILE__ << "\n" \
                << "Line: " << __LINE__; \
            LOG_ERROR(oss.str()); \
            DEBUG_BREAK(); \
        } \
    } while(0)

// 检查索引范围
#define CHECK_RANGE(index, size) \
    do { \
        if ((index) >= (size)) { \
            std::ostringstream oss; \
            oss << "\n!!! INDEX OUT OF RANGE !!!\n" \
                << "Index: " << (index) << "\n" \
                << "Size: " << (size) << "\n" \
                << "File: " << __FILE__ << "\n" \
                << "Line: " << __LINE__; \
            LOG_ERROR(oss.str()); \
            DEBUG_BREAK(); \
        } \
    } while(0)

// 不可达代码断言
#define UNREACHABLE() \
    do { \
        std::ostringstream oss; \
        oss << "\n!!! UNREACHABLE CODE REACHED !!!\n" \
            << "File: " << __FILE__ << "\n" \
            << "Line: " << __LINE__; \
        LOG_ERROR(oss.str()); \
        DEBUG_BREAK(); \
    } while(0)