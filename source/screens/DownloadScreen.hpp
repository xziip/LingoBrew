#pragma once
#include "Screen.hpp"
#include "utils/DownloadManager.hpp"

class DownloadScreen : public Screen {
public:
    DownloadScreen();
    ~DownloadScreen() override;

    void Draw() override;
    bool Update(Input &input) override;

private:
    enum DownloadScreenState {
        STATE_SELECT_REGION,     // 选择系统区域
        STATE_SELECT_MIRROR,     // 选择镜像源类型
        STATE_SELECT_LANGUAGE,   // 选择要下载的语言
        STATE_DOWNLOADING,       // 下载中
        STATE_EXTRACTING,        // 解压中
        STATE_COMPLETE,          // 完成（询问是否复制）
        STATE_ERROR              // 错误
    };
    
    enum SystemRegion {
        REGION_EU = 0,
        REGION_US = 1,
        REGION_JP = 2,
        REGION_COUNT = 3
    };
    
    enum MirrorType {
        MIRROR_ORIGINAL = 0,     // 原镜像
        MIRROR_ACCELERATED = 1,  // 加速镜像
        MIRROR_COUNT = 2
    };
    
    enum TargetLanguage {
        LANG_CHINESE = 0,        // 当前只有中文
        LANG_COUNT = 1
    };
    
    DownloadScreenState mScreenState;
    DownloadManager mDownloadManager;
    
    SystemRegion mSelectedRegion;
    MirrorType mSelectedMirror;
    TargetLanguage mSelectedLanguage;
    
    std::string mStatusMessage;
    float mProgress;
    long mDownloadedSize;
    long mTotalSize;
    long mSpeed;
    
    bool mShowCancelConfirm;  // 是否显示取消确认对话框
    bool mSHA256Verified;     // 是否进行了 SHA-256 校验
    
    const int mDefaultXPos     = 32;
    const int mDefaultYPos     = 128;
    const int mDefaultFontSize = 56;
    
    // 生成下载URL
    std::string GenerateDownloadUrl();
    
    // 开始下载
    void StartDownload();
    
    // 格式化文件大小显示
    std::string FormatFileSize(long bytes);
    // 格式化速度显示
    std::string FormatSpeed(long bytesPerSec);
    
    // 获取区域名称
    std::string GetRegionName(SystemRegion region);
    // 获取镜像类型名称
    std::string GetMirrorTypeName(MirrorType type);
    // 获取语言名称
    std::string GetLanguageName(TargetLanguage lang);
    
    // 下载并解析 SHA256.txt
    std::string DownloadAndParseSHA256();
    // 解析 SHA256.txt 文件
    std::string ParseSHA256File(const std::string& filePath);
};
