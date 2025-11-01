#pragma once

#include "Screen.hpp"

class DebugScreen : public Screen {
public:
    DebugScreen();
    ~DebugScreen() override;

    void Draw() override;
    bool Update(Input &input) override;

private:
    enum DebugOption {
        DEBUG_OPTION_LOGGING,
        DEBUG_OPTION_VERBOSE,
        DEBUG_OPTION_DEFAULT_PAGE,
        DEBUG_OPTION_CLEAR_LOGS,
        DEBUG_OPTION_HIDE_MENU,
        DEBUG_OPTION_BACK,
        
        DEBUG_OPTION_MIN = DEBUG_OPTION_LOGGING,
        DEBUG_OPTION_MAX = DEBUG_OPTION_BACK
    };
    
    struct DebugEntry {
        uint16_t icon;
        const char* name;
    };
    
    DebugOption mSelectedOption;
    int mCurrentPage;  // 当前页面 (0 或 1)
    bool mShowClearConfirm;  // 是否显示清除日志确认对话框
    
    const int mCardHeight = 150;
    const int mCardSpacing = 20;
    const int mStartY = 128;
    const int mIconSize = 68;
    const int mItemsPerPage = 4;  // 每页显示4个选项
};
