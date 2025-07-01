# if 0
// ======================
// 示例使用
// ======================

#include <vector>

#include "Utility/assertions.h"
#include "Utility/logging.h"

class Resource {
public:
    Resource(const std::string& path) : path(path) {
        LOG_INFO("加载资源: " + path);
        // 模拟资源加载失败
        if (path.find("invalid") != std::string::npos) {
            LOG_ERROR("资源加载失败: " + path);
        }
    }

    void use() {
        LOG_INFO("使用资源: " + path);
        CHECK_MSG(!path.empty(), "资源路径不能为空");
    }

private:
    std::string path;
};

void processValue(int value) {
    LOG_DEBUG("处理值: " + std::to_string(value));
    
    CHECK(value >= 0 && value <= 100);
    
    if (value < 50) {
        LOG_INFO("值小于50");
    } else if (value < 80) {
        LOG_INFO("值在50-79之间");
    } else if (value <= 100) {
        LOG_INFO("值在80-100之间");
    } else {
        // 这个分支理论上不应该到达，因为CHECK已经验证了范围
        UNREACHABLE();
    }
}

void demoChecks() {
    // 基本检查
    CHECK(2 + 2 == 4);
    
    // 带消息的检查
    int x = 42;
    CHECK_MSG(x > 0, "i Test");
    // CHECK_MSG(x > 0, "x 应该是正数");
    
    // 指针检查
    int* ptr = nullptr;
    // 取消注释下面一行会触发断言
    CHECK_PTR(ptr);
    
    // 范围检查
    std::vector<int> numbers = {1, 2, 3};
    int index = 3;
    // 取消注释下面一行会触发断言
    CHECK_RANGE(index, numbers.size());
}

int main() {
    // 配置日志系统
    Logger::get().setLogFile("application.log");
    Logger::get().setMinLevel(LogLevel::Debug);
    
    LOG_INFO("======= 应用程序启动 =======");
    
    try {
        Resource validRes("valid_resource.txt");
        validRes.use();
        
        Resource invalidRes("invalid_resource.dat");
        invalidRes.use(); // 这里会触发断言
        
        // 测试不同值处理
        processValue(42);
        processValue(75);
        processValue(95);
        processValue(150); // 这里会触发断言
        
        // 演示其他检查类型
        demoChecks();
    }
    catch (const std::exception& e) {
        LOG_ERROR("异常捕获: " + std::string(e.what()));
    }
    
    LOG_INFO("======= 应用程序结束 =======");
    return 0;
}
# endif