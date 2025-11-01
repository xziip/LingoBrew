#pragma once
#include "Screen.hpp"
#include "common.h"
#include "utils/BackupManager.hpp"
#include <dirent.h>
#include <queue>

class BackupScreen : public Screen {
public:
    BackupScreen();
    ~BackupScreen() override;

    void Draw() override;
    bool Update(Input &input) override;

private:
    enum BackupState {
        BACKUP_STATE_SELECT_STORAGE,  // 选择存储类型 (MLC/SLC)
        BACKUP_STATE_SELECT,          // 选择备份目标
        BACKUP_STATE_SELECT_MODE,     // 选择备份模式（仅title和fonts）
        BACKUP_STATE_SELECT_FULL,     // 选择完整备份目录（sys或usr）
        BACKUP_STATE_SELECT_SLC_TYPE, // 选择 SLC 类型 (slc/slccmpt)
        BACKUP_STATE_DO_BACKUP,       // 执行备份
        BACKUP_STATE_DONE,            // 完成
        BACKUP_STATE_ERROR,           // 错误
    };
    BackupState mBackupState = BACKUP_STATE_SELECT_STORAGE;
    
    enum StorageType {
        STORAGE_TYPE_MLC,   // MLC 存储
        STORAGE_TYPE_SLC,   // SLC 存储
    };
    StorageType mStorageType = STORAGE_TYPE_MLC;
    
    enum SlcType {
        SLC_TYPE_SLCCMPT,   // SLCCMPT
        SLC_TYPE_SLC,       // SLC
    };
    SlcType mSlcType = SLC_TYPE_SLCCMPT;
    
    enum BackupTarget {
        BACKUP_TARGET_TITLE,    // 备份title文件夹
        BACKUP_TARGET_FONTS,    // 备份字体
        BACKUP_TARGET_FULL,     // 完整MLC备份
    };
    BackupTarget mBackupTarget = BACKUP_TARGET_TITLE;
    
    enum BackupMode {
        BACKUP_MODE_ALL,        // 全量备份
        BACKUP_MODE_SELECTIVE   // 选择性备份
    };
    BackupMode mBackupMode = BACKUP_MODE_ALL;
    
    enum FullBackupTarget {
        FULL_BACKUP_SYS,        // 备份sys文件夹
        FULL_BACKUP_USR,        // 备份usr文件夹
    };
    FullBackupTarget mFullBackupTarget = FULL_BACKUP_SYS;

    std::string mError = "未知错误";
    
    // 备份管理器
    BackupManager mBackupManager;

    const int mDefaultXPos     = 32;
    const int mDefaultYPos     = 128;
    const int mDefaultFontSize = 56;

    void DrawSimpleText(const std::string &text) const;
    void DrawSimpleText(const std::vector<std::string> &texts) const;
};
