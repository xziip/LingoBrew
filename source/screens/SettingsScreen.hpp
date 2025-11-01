#pragma once

#include "Screen.hpp"
#include <memory>

class SettingsScreen : public Screen {
public:
    SettingsScreen();
    ~SettingsScreen() override;

    void Draw() override;
    bool Update(Input &input) override;

private:
    std::unique_ptr<Screen> mSubscreen;  // 用于跳转到下载页面
    
    enum SettingsOption {
        SETTINGS_OPTION_LANGUAGE,
        SETTINGS_OPTION_SHOW_COPY_DETECT,  // 新增：显示文件检测提醒
        SETTINGS_OPTION_DOWNLOAD,  // 新增：下载语言文件
        SETTINGS_OPTION_CLEAR_TITLES,  // 新增：清除SD卡title文件夹
        SETTINGS_OPTION_BACK,
        
        SETTINGS_OPTION_MIN = SETTINGS_OPTION_LANGUAGE,
        SETTINGS_OPTION_MAX = SETTINGS_OPTION_BACK
    };
    
    struct SettingsEntry {
        uint16_t icon;
        const char* key;  // 语言文件键名
    };
    
    SettingsOption mSelectedOption;
    bool mShowClearConfirm;  // 是否显示清除Title文件夹确认对话框
    
    const int mCardHeight = 150;
    const int mCardSpacing = 20;
    const int mStartY = 128;
    const int mIconSize = 68;
};
