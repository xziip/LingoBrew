#include "FileLogger.hpp"
#include "Config.hpp"
#include "Utils.hpp"
#include <cstdarg>
#include <ctime>
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>

FileLogger::FileLogger() 
    : mLogFile(nullptr) {
}

FileLogger::~FileLogger() {
    EndLog();
}

FileLogger& FileLogger::GetInstance() {
    static FileLogger instance;
    return instance;
}

bool FileLogger::IsEnabled() const {
    return Config::GetInstance().IsLoggingEnabled();
}

int FileLogger::GetNextLogNumber(const char* prefix) {
    const char* logDir = "fs:/vol/external01/log/lingobrew";
    DIR* dir = opendir(logDir);
    if (!dir) {
        return 0;
    }
    
    int maxNumber = -1;
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        
        // 检查文件名格式: prefix##.log
        size_t prefixLen = strlen(prefix);
        if (name.length() >= prefixLen + 6 && // prefix + "##.log"
            name.substr(0, prefixLen) == prefix &&
            name.substr(name.length() - 4) == ".log") {
            
            // 提取数字部分
            std::string numStr = name.substr(prefixLen, name.length() - prefixLen - 4);
            if (numStr.length() == 2 && isdigit(numStr[0]) && isdigit(numStr[1])) {
                int num = (numStr[0] - '0') * 10 + (numStr[1] - '0');
                if (num > maxNumber) {
                    maxNumber = num;
                }
            }
        }
    }
    
    closedir(dir);
    return maxNumber + 1;
}

bool FileLogger::StartLog(const char* prefix) {
    if (!IsEnabled()) {
        return false;
    }
    
    // 结束之前的日志
    EndLog();
    
    // 创建日志目录
    const char* logDir = "fs:/vol/external01/log/lingobrew";
    if (!Utils::CreateSubfolder(logDir)) {
        return false;
    }
    
    // 获取下一个日志编号
    int logNum = GetNextLogNumber(prefix);
    if (logNum > 99) {
        logNum = 0; // 重置到 00
    }
    
    // 创建日志文件路径
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s%02d.log", logDir, prefix, logNum);
    mCurrentLogPath = filename;
    
    // 打开日志文件
    mLogFile = fopen(filename, "w");
    if (!mLogFile) {
        return false;
    }
    
    // 写入日志头
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    fprintf(mLogFile, "========================================\n");
    fprintf(mLogFile, "LingoBrew Log - %s\n", prefix);
    fprintf(mLogFile, "Time: %s\n", timebuf);
    fprintf(mLogFile, "========================================\n\n");
    fflush(mLogFile);
    
    return true;
}

void FileLogger::WriteLog(const char* level, const char* format, va_list args) {
    if (!mLogFile) {
        return;
    }
    
    // 获取时间戳
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%H:%M:%S", timeinfo);
    
    // 写入级别和时间
    fprintf(mLogFile, "[%s][%s] ", timebuf, level);
    
    // 写入消息
    vfprintf(mLogFile, format, args);
    fprintf(mLogFile, "\n");
    fflush(mLogFile);
}

void FileLogger::Log(const char* format, ...) {
    if (!mLogFile) return;
    
    va_list args;
    va_start(args, format);
    WriteLog("INFO", format, args);
    va_end(args);
}

void FileLogger::LogError(const char* format, ...) {
    if (!mLogFile) return;
    
    va_list args;
    va_start(args, format);
    WriteLog("ERROR", format, args);
    va_end(args);
}

void FileLogger::LogSuccess(const char* format, ...) {
    if (!mLogFile) return;
    
    va_list args;
    va_start(args, format);
    WriteLog("SUCCESS", format, args);
    va_end(args);
}

void FileLogger::LogFileOperation(const char* srcPath, const char* dstPath) {
    if (!mLogFile || !Config::GetInstance().IsVerboseLogging()) {
        return;
    }
    
    Log("复制: %s -> %s", srcPath, dstPath);
}

void FileLogger::LogBackupStart(const char* srcPath, const char* dstPath) {
    if (!mLogFile) return;
    
    fprintf(mLogFile, "\n");
    Log("备份开始");
    Log("源路径: %s", srcPath);
    Log("目标路径: %s", dstPath);
    fprintf(mLogFile, "\n");
    fflush(mLogFile);
}

void FileLogger::LogBackupEnd(int total, int skipped) {
    if (!mLogFile) return;
    
    fprintf(mLogFile, "\n");
    Log("备份完成");
    Log("总计项目: %d", total);
    if (skipped > 0) {
        Log("跳过文件: %d", skipped);
    }
    fprintf(mLogFile, "\n");
    fflush(mLogFile);
}

void FileLogger::EndLog() {
    if (mLogFile) {
        fprintf(mLogFile, "\n========================================\n");
        fprintf(mLogFile, "日志结束\n");
        fprintf(mLogFile, "========================================\n");
        fclose(mLogFile);
        mLogFile = nullptr;
    }
}
