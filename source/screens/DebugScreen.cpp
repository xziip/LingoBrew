#include "DebugScreen.hpp"
#include "Gfx.hpp"
#include "../utils/Config.hpp"
#include "../utils/FileLogger.hpp"
#include "../utils/Utils.hpp"
#include "../utils/logger.h"
#include "../utils/Language.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <cstdio>

DebugScreen::DebugScreen() 
    : mSelectedOption(DEBUG_OPTION_LOGGING)
    , mCurrentPage(0)
    , mShowClearConfirm(false) {
}

DebugScreen::~DebugScreen() {
}

void DebugScreen::Draw() {
    Language& lang = Language::GetInstance();
    DrawTopBar(lang.Get("menu.debug"));
    
    // 定义调试选项图标
    uint32_t icons[] = {
        0xf03a,  // file-text icon - 日志功能
        0xf15c,  // file icon - 详细日志
        0xf015,  // home icon - 默认页面
        0xf1f8,  // trash icon - 清除日志
        0xf070,  // eye-slash icon - 隐藏菜单
        0xf060   // arrow-left icon - 返回
    };
    
    int yPos = mStartY;
    
    // 计算当前页面的起始和结束索引
    int startIdx = mCurrentPage * mItemsPerPage;
    int endIdx = startIdx + mItemsPerPage - 1;
    if (endIdx > DEBUG_OPTION_MAX) {
        endIdx = DEBUG_OPTION_MAX;
    }
    
    // 只绘制当前页面的选项
    for (int i = startIdx; i <= endIdx; i++) {
        bool isSelected = (i == mSelectedOption);
        
        // 绘制卡片背景
        if (isSelected) {
            Gfx::DrawRectFilled(32, yPos, 1216, mCardHeight, Gfx::COLOR_ALT_BACKGROUND);
            Gfx::DrawRect(32, yPos, 1216, mCardHeight, 8, Gfx::COLOR_HIGHLIGHTED);
        } else {
            Gfx::DrawRectFilled(32, yPos, 1216, mCardHeight, Gfx::COLOR_ALT_BACKGROUND);
        }
        
        // 绘制图标
        Gfx::DrawIcon(68, yPos + (mCardHeight / 2), mIconSize, Gfx::COLOR_TEXT, icons[i], Gfx::ALIGN_VERTICAL);
        
        // 绘制主文本
        std::string mainText;
        const char* onText = lang.Get("button.on");
        const char* offText = lang.Get("button.off");
        
        switch (i) {
            case DEBUG_OPTION_LOGGING:
                mainText = lang.Get("debug.logging");
                mainText += ": ";
                mainText += Config::GetInstance().IsLoggingEnabled() ? onText : offText;
                break;
            case DEBUG_OPTION_VERBOSE:
                mainText = lang.Get("debug.verbose");
                mainText += ": ";
                mainText += Config::GetInstance().IsVerboseLogging() ? onText : offText;
                break;
            case DEBUG_OPTION_DEFAULT_PAGE: {
                const char* pageKeys[] = { 
                    "page.main", "page.copy", "page.backup", "page.restore", 
                    "page.reboot", "page.settings", "page.debug", "page.about", "page.download"
                };
                int page = Config::GetInstance().GetDefaultStartPage();
                if (page < 0 || page > 8) {
                    page = 0;
                }
                mainText = lang.Get("debug.defaultpage");
                mainText += ": ";
                mainText += lang.Get(pageKeys[page]);
                break;
            }
            case DEBUG_OPTION_CLEAR_LOGS:
                mainText = lang.Get("debug.clearlogs");
                break;
            case DEBUG_OPTION_HIDE_MENU:
                mainText = lang.Get("debug.hidemenu");
                break;
            case DEBUG_OPTION_BACK:
                mainText = lang.Get("button.back");
                break;
        }
        
        Gfx::Print(180, yPos + 45, 60, Gfx::COLOR_TEXT, mainText, Gfx::ALIGN_VERTICAL);
        
        // 绘制描述文本
        const char* descText = "";
        switch (i) {
            case DEBUG_OPTION_LOGGING:
                descText = Config::GetInstance().IsLoggingEnabled() ? 
                    lang.Get("debug.logging.on.desc") : lang.Get("debug.logging.off.desc");
                break;
            case DEBUG_OPTION_VERBOSE:
                descText = Config::GetInstance().IsVerboseLogging() ? 
                    lang.Get("debug.verbose.on.desc") : lang.Get("debug.verbose.off.desc");
                break;
            case DEBUG_OPTION_DEFAULT_PAGE:
                descText = lang.Get("debug.defaultpage.desc");
                break;
            case DEBUG_OPTION_CLEAR_LOGS:
                descText = lang.Get("debug.clearlogs.desc");
                break;
            case DEBUG_OPTION_HIDE_MENU:
                descText = lang.Get("debug.hidemenu.desc");
                break;
            case DEBUG_OPTION_BACK:
                descText = lang.Get("debug.back.desc");
                break;
        }
        
        Gfx::Print(180, yPos + 100, 40, Gfx::COLOR_TEXT, descText, Gfx::ALIGN_VERTICAL);
        
        yPos += mCardHeight + mCardSpacing;
    }
    
    // 显示分页提示
    int totalPages = (DEBUG_OPTION_MAX + mItemsPerPage) / mItemsPerPage;
    std::string pageInfo = lang.Get("debug.page");
    pageInfo += " " + std::to_string(mCurrentPage + 1) + "/" + std::to_string(totalPages);
    
    // 如果显示确认对话框
    if (mShowClearConfirm) {
        // 绘制半透明背景
        Gfx::DrawRectFilled(0, 0, Gfx::SCREEN_WIDTH, Gfx::SCREEN_HEIGHT, {0, 0, 0, 200});
        
        // 绘制对话框
        int dialogW = 800;
        int dialogH = 300;
        int dialogX = (Gfx::SCREEN_WIDTH - dialogW) / 2;
        int dialogY = (Gfx::SCREEN_HEIGHT - dialogH) / 2;
        
        Gfx::DrawRectFilled(dialogX, dialogY, dialogW, dialogH, Gfx::COLOR_ALT_BACKGROUND);
        Gfx::DrawRect(dialogX, dialogY, dialogW, dialogH, 4, Gfx::COLOR_HIGHLIGHTED);
        
        // 绘制警告图标和文字 (图标往上移动)
        Gfx::DrawIcon(dialogX + dialogW/2, dialogY + 60, 80, Gfx::COLOR_WARNING, 0xf071, Gfx::ALIGN_HORIZONTAL);
        Gfx::Print(dialogX + dialogW/2, dialogY + 150, 50, Gfx::COLOR_TEXT, lang.Get("dialog.confirm.delete"), Gfx::ALIGN_HORIZONTAL);
        Gfx::Print(dialogX + dialogW/2, dialogY + 215, 40, Gfx::COLOR_ALT_TEXT, lang.Get("dialog.warning.irreversible"), Gfx::ALIGN_HORIZONTAL);
        
        std::string confirmBtn = std::string("\ue000 ") + lang.Get("button.confirm");
        std::string cancelBtn = std::string("\ue001 ") + lang.Get("button.cancel");
        DrawBottomBar(confirmBtn.c_str(), nullptr, cancelBtn.c_str());
    } else {
        std::string navText = std::string("\ue07d ") + lang.Get("button.select") + 
                             "  \ue000 " + lang.Get("button.confirm") + 
                             "  \ue001 " + lang.Get("button.back");
        std::string pageText = std::string("\ue052/\ue053 ") + lang.Get("button.page");
        DrawBottomBar(navText.c_str(), pageInfo.c_str(), pageText.c_str());
    }
}

bool DebugScreen::Update(Input &input) {
    // 如果显示确认对话框,只处理确认/取消
    if (mShowClearConfirm) {
        if (input.data.buttons_d & Input::BUTTON_A) {
            // 确认删除
            const char* logDir = "fs:/vol/external01/log/lingobrew";
            DIR* dir = opendir(logDir);
            if (dir) {
                struct dirent* entry;
                while ((entry = readdir(dir)) != nullptr) {
                    std::string name = entry->d_name;
                    if (name != "." && name != ".." && name.find(".log") != std::string::npos) {
                        std::string path = std::string(logDir) + "/" + name;
                        remove(path.c_str());
                    }
                }
                closedir(dir);
            }
            mShowClearConfirm = false;
            return true;
        } else if (input.data.buttons_d & Input::BUTTON_B) {
            // 取消
            mShowClearConfirm = false;
            return true;
        }
        return true;
    }
    
    // 计算当前页面范围
    int startIdx = mCurrentPage * mItemsPerPage;
    int endIdx = startIdx + mItemsPerPage - 1;
    if (endIdx > DEBUG_OPTION_MAX) {
        endIdx = DEBUG_OPTION_MAX;
    }
    
    // L键: 上一页
    if (input.data.buttons_d & Input::BUTTON_L) {
        if (mCurrentPage > 0) {
            mCurrentPage--;
            // 调整选中项到新页面的第一项
            mSelectedOption = (DebugOption)(mCurrentPage * mItemsPerPage);
        }
        return true;
    }
    
    // R键: 下一页
    if (input.data.buttons_d & Input::BUTTON_R) {
        int totalPages = (DEBUG_OPTION_MAX + mItemsPerPage) / mItemsPerPage;
        if (mCurrentPage < totalPages - 1) {
            mCurrentPage++;
            // 调整选中项到新页面的第一项
            int newStart = mCurrentPage * mItemsPerPage;
            mSelectedOption = (DebugOption)newStart;
        }
        return true;
    }
    
    if (input.data.buttons_d & Input::BUTTON_UP) {
        if (mSelectedOption > startIdx) {
            mSelectedOption = (DebugOption)(mSelectedOption - 1);
        } else {
            // 到达当前页顶部,跳到当前页底部
            mSelectedOption = (DebugOption)endIdx;
        }
    } else if (input.data.buttons_d & Input::BUTTON_DOWN) {
        if (mSelectedOption < endIdx) {
            mSelectedOption = (DebugOption)(mSelectedOption + 1);
        } else {
            // 到达当前页底部,跳到当前页顶部
            mSelectedOption = (DebugOption)startIdx;
        }
    } else if (input.data.buttons_d & Input::BUTTON_A) {
        switch (mSelectedOption) {
            case DEBUG_OPTION_LOGGING: {
                // 切换日志功能
                bool current = Config::GetInstance().IsLoggingEnabled();
                Config::GetInstance().SetLoggingEnabled(!current);
                break;
            }
            case DEBUG_OPTION_VERBOSE: {
                // 切换详细日志功能
                bool current = Config::GetInstance().IsVerboseLogging();
                Config::GetInstance().SetVerboseLogging(!current);
                break;
            }
            case DEBUG_OPTION_DEFAULT_PAGE: {
                // 切换默认启动页面 (0=主菜单, 1=复制Title, 2=备份, 3=恢复, 4=重启, 5=设置, 6=Debug, 7=关于, 8=下载)
                int current = Config::GetInstance().GetDefaultStartPage();
                current = (current + 1) % 9;  // 循环切换 0-8
                Config::GetInstance().SetDefaultStartPage(current);
                
                // 调试输出
                WHBLogPrintf("默认页面切换到: %d", current);
                
                break;
            }
            case DEBUG_OPTION_CLEAR_LOGS: {
                // 显示确认对话框
                mShowClearConfirm = true;
                break;
            }
            case DEBUG_OPTION_HIDE_MENU: {
                // 隐藏 Debug 菜单
                Config::GetInstance().SetDebugMenuVisible(false);
                return false; // 返回菜单
            }
            case DEBUG_OPTION_BACK: {
                return false; // 返回菜单
            }
        }
    } else if (input.data.buttons_d & Input::BUTTON_B) {
        return false; // 返回菜单
    }
    
    return true;
}
