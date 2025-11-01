#pragma once
#include <string>

class Config {
public:
    static Config& GetInstance();
    
    // 日志设置
    bool IsLoggingEnabled() const { return mLoggingEnabled; }
    void SetLoggingEnabled(bool enabled);
    
    bool IsVerboseLogging() const { return mVerboseLogging; }
    void SetVerboseLogging(bool verbose);
    
    // Debug 菜单设置
    bool IsDebugMenuVisible() const { return mDebugMenuVisible; }
    void SetDebugMenuVisible(bool visible);
    
    // 默认启动页面设置
    int GetDefaultStartPage() const { return mDefaultStartPage; }
    void SetDefaultStartPage(int page);
    
    // 语言设置
    int GetLanguage() const { return mLanguage; }
    void SetLanguage(int lang);
    
    // 文件检测提醒设置
    bool ShowCopyFileDetection() const { return mShowCopyFileDetection; }
    void SetShowCopyFileDetection(bool show);
    
    // 加载/保存配置
    bool Load();
    bool Save();
    
private:
    Config();
    ~Config();
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    bool mLoggingEnabled;
    bool mVerboseLogging;
    bool mDebugMenuVisible;
    int mDefaultStartPage;  // 0=主菜单, 1=复制Title, 2=备份, 3=恢复, 4=重启, 5=设置, 6=Debug, 7=关于, 8=下载
    int mLanguage;  // 0=中文, 1=English, 2=日本語
    bool mShowCopyFileDetection;  // 是否显示复制文件检测提醒
    std::string mConfigPath;
};
