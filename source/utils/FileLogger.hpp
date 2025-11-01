#pragma once
#include <string>
#include <cstdio>

class FileLogger {
public:
    static FileLogger& GetInstance();
    
    // 开始一个新的日志会话
    bool StartLog(const char* prefix); // 例如 "slcbackup", "mlcbackup"
    
    // 记录日志
    void Log(const char* format, ...);
    void LogError(const char* format, ...);
    void LogSuccess(const char* format, ...);
    void LogFileOperation(const char* srcPath, const char* dstPath);
    void LogBackupStart(const char* srcPath, const char* dstPath);
    void LogBackupEnd(int total, int skipped);
    
    // 结束日志会话
    void EndLog();
    
    // 检查日志是否启用
    bool IsEnabled() const;
    
private:
    FileLogger();
    ~FileLogger();
    FileLogger(const FileLogger&) = delete;
    FileLogger& operator=(const FileLogger&) = delete;
    
    void WriteLog(const char* level, const char* format, va_list args);
    int GetNextLogNumber(const char* prefix);
    
    FILE* mLogFile;
    std::string mCurrentLogPath;
};
