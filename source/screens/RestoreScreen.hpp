#pragma once

#include "Screen.hpp"
#include "common.h"
#include "../utils/BackupManager.hpp"
#include <string>
#include <vector>

class RestoreScreen : public Screen {
public:
    RestoreScreen();
    ~RestoreScreen() override;

    void Draw() override;
    bool Update(Input &input) override;

private:
    enum StorageType {
        STORAGE_TYPE_MLC,
        STORAGE_TYPE_SLC
    };

    enum SlcType {
        SLC_TYPE_SLCCMPT,
        SLC_TYPE_SLC
    };

    enum RestoreTarget {
        RESTORE_TARGET_TITLE,
        RESTORE_TARGET_PLUGIN
    };

    enum RestoreState {
        RESTORE_STATE_SELECT_STORAGE,  // 选择存储类型 (MLC/SLC)
        RESTORE_STATE_SELECT_SLC_TYPE, // 选择 SLC 类型 (SLCCMPT/SLC)
        RESTORE_STATE_ASK_TARGET,      // 询问恢复什么（Title或字体）
        RESTORE_STATE_DO_RESTORE,       // 执行恢复
        RESTORE_STATE_DONE,             // 恢复完成
        RESTORE_STATE_ASK_REBOOT,       // 询问是否重启
        RESTORE_STATE_ERROR             // 错误
    };

    RestoreState mState;
    StorageType mStorageType;
    SlcType mSlcType = SLC_TYPE_SLCCMPT;
    RestoreTarget mRestoreTarget;
    BackupManager mBackupManager;
    std::string mError;

    const int mDefaultXPos     = 32;
    const int mDefaultYPos     = 128;
    const int mDefaultFontSize = 56;

    void DrawSimpleText(const std::vector<std::string>& lines);
};
