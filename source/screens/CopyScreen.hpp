#pragma once
#include "Screen.hpp"
#include "common.h"
#include "utils/BackupManager.hpp"
#include <dirent.h>
#include <queue>
#include <memory>

class CopyScreen : public Screen {
public:
    CopyScreen();
    ~CopyScreen() override;

    void Draw() override;

    bool Update(Input &input) override;

private:
    std::unique_ptr<Screen> mSubscreen;  // 用于跳转到下载页面
    
    enum CopyState {
        COPY_STATE_CHECKING_FILES,     // 检测SD卡文件
        COPY_STATE_HAS_LANG_FILES,     // 已有语言文件,询问是否下载
        COPY_STATE_HAS_OTHER_FILES,    // 有其他文件但无语言文件,询问是否复制
        COPY_STATE_NO_FILES,           // 没有任何文件,询问是否下载
        COPY_STATE_SELECT,
        COPY_STATE_ERROR,
        COPY_STATE_ASK_BACKUP_MODE,   // 询问备份模式
        COPY_STATE_DO_BACKUP,         // 执行备份
        COPY_STATE_CREATE_DIRECTORY,
        COPY_STATE_OPEN_DIR,
        COPY_STATE_COPY_FILE,
        COPY_STATE_CLEANUP,
        COPY_STATE_ASK_PLUGIN,        // 询问是否复制字体
        COPY_STATE_ASK_PLUGIN_BACKUP, // 询问是否备份字体
        COPY_STATE_DO_PLUGIN_BACKUP,  // 执行字体备份
        COPY_STATE_DONE,
    };
    CopyState mCopyState = COPY_STATE_CHECKING_FILES;
    
    enum BackupMode {
        BACKUP_MODE_NONE,       // 不备份
        BACKUP_MODE_ALL,        // 备份整个MLC
        BACKUP_MODE_OVERWRITE   // 只备份将被覆盖的文件
    };
    BackupMode mBackupMode = BACKUP_MODE_NONE;

    std::string mError = "未知错误";
    std::string mDestinationDirectory = HAX_DESTINATION_PATH;
    std::string mSourceDirectory = HAX_SOURCE_PATH;

    struct FileEntry {
        std::string path;
        bool isDirectory;
        FileEntry(const std::string& p, bool isDir) : path(p), isDirectory(isDir) {}
    };
    
    std::queue<FileEntry> mPendingFiles;  // 待复制的文件队列
    std::string mCurrentFile;             // 当前正在复制的文件
    bool mCurrentIsDirectory = false;
    int mTotalItems = 0;                  // 总项目数（文件+目录）
    int mProcessedItems = 0;              // 已处理项目数
    bool mIsScanning = false;             // 是否正在扫描文件
    
    // 分块文件复制相关
    FILE* mCopySourceFile = nullptr;      // 源文件句柄
    FILE* mCopyDestFile = nullptr;        // 目标文件句柄
    long mCopyFileSize = 0;               // 当前文件总大小
    long mCopyFileCopied = 0;             // 当前文件已复制大小
    bool mIsCopyingFile = false;          // 是否正在复制文件中
    
    // 备份管理器
    BackupManager mBackupManager;
    
    // 文件检测相关
    bool mHasLanguageFiles = false;        // SD卡上是否有语言文件
    bool mHasOtherFiles = false;           // SD卡上是否有其他文件
    std::string mDetectedRegion;           // 检测到的区域
    bool mErrorLogged = false;             // 是否已记录错误日志
    
    // 检测SD卡上是否有语言文件
    bool CheckLanguageFiles();
    
    // 扫描目录并添加文件到队列
    void ScanDirectory(const std::string& srcPath, const std::string& dstPath);
    // 复制单个文件或创建目录
    bool ProcessNextFile();
    // 开始文件复制（打开文件）
    bool StartFileCopy(const std::string& srcPath, const std::string& dstPath);
    // 继续文件复制（复制一块数据）
    bool ContinueFileCopy();
    // 结束文件复制（关闭文件）
    void EndFileCopy();

    const int mDefaultXPos     = 32;
    const int mDefaultYPos     = 128;
    const int mDefaultFontSize = 56;

    void DrawSimpleText(const std::string &text) const;
    void DrawSimpleText(const std::vector<std::string> &texts) const;
};
