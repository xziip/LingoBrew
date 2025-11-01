#include "SettingsScreen.hpp"
#include "DownloadScreen.hpp"
#include "Gfx.hpp"
#include "../utils/Config.hpp"
#include "../utils/Language.hpp"
#include "../utils/logger.h"
#include <dirent.h>
#include <sys/stat.h>
#include <cstdio>
#include <unistd.h>

SettingsScreen::SettingsScreen() 
    : mSelectedOption(SETTINGS_OPTION_LANGUAGE)
    , mShowClearConfirm(false) {
}

SettingsScreen::~SettingsScreen() {
}

void SettingsScreen::Draw() {
    // 如果有子屏幕，绘制子屏幕
    if (mSubscreen) {
        mSubscreen->Draw();
        return;
    }
    
    Language& lang = Language::GetInstance();
    DrawTopBar(lang.Get("settings.title"));
    
    // 定义设置选项
    SettingsEntry entries[] = {
        { 0xf0ac, "settings.language" },           // globe icon
        { 0xf06e, "settings.show_copy_detect" },   // eye icon
        { 0xf019, "settings.download" },           // download icon
        { 0xf1f8, "settings.clear_titles" },       // trash icon - 清除title文件夹
        { 0xf060, "settings.back" }                // arrow-left icon
    };
    
    int yPos = mStartY;
    
    for (int i = SETTINGS_OPTION_MIN; i <= SETTINGS_OPTION_MAX; i++) {
        bool isSelected = (i == mSelectedOption);
        
        // 绘制卡片背景
        if (isSelected) {
            Gfx::DrawRectFilled(32, yPos, 1216, mCardHeight, Gfx::COLOR_ALT_BACKGROUND);
            Gfx::DrawRect(32, yPos, 1216, mCardHeight, 8, Gfx::COLOR_HIGHLIGHTED);
        } else {
            Gfx::DrawRectFilled(32, yPos, 1216, mCardHeight, Gfx::COLOR_ALT_BACKGROUND);
        }
        
        // 绘制图标
        Gfx::DrawIcon(68, yPos + (mCardHeight / 2), mIconSize, Gfx::COLOR_TEXT, entries[i].icon, Gfx::ALIGN_VERTICAL);
        
        // 绘制主文本
        std::string mainText = lang.Get(entries[i].key);
        
        // 对于语言选项,显示当前语言
        if (i == SETTINGS_OPTION_LANGUAGE) {
            int currentLang = Config::GetInstance().GetLanguage();
            mainText += ": ";
            mainText += lang.GetLanguageName(static_cast<LanguageCode>(currentLang));
        } else if (i == SETTINGS_OPTION_SHOW_COPY_DETECT) {
            // 显示文件检测提醒的状态
            bool showDetect = Config::GetInstance().ShowCopyFileDetection();
            mainText += ": ";
            mainText += showDetect ? lang.Get("common.enabled") : lang.Get("common.disabled");
        }
        
        Gfx::Print(180, yPos + 45, 60, Gfx::COLOR_TEXT, mainText, Gfx::ALIGN_VERTICAL);
        
        // 绘制描述文本
        std::string descText;
        
        // 对于语言选项，显示下一个语言的描述文本
        if (i == SETTINGS_OPTION_LANGUAGE) {
            int currentLang = Config::GetInstance().GetLanguage();
            int nextLang = (currentLang + 1) % 3;  // 下一个语言
            
            // 根据下一个语言显示对应的描述
            switch (nextLang) {
                case 0: // 中文
                    descText = "切换程序显示语言 / Change program language";
                    break;
                case 1: // English
                    descText = "Change program language / プログラム表示言語を変更";
                    break;
                case 2: // 日本語
                    descText = "プログラム表示言語を変更 / 切换程序显示语言";
                    break;
            }
        } else if (i == SETTINGS_OPTION_DOWNLOAD) {
            // 下载选项的描述
            descText = lang.Get("settings.download.desc");
        } else {
            std::string descKey = std::string(entries[i].key) + ".desc";
            descText = lang.Get(descKey.c_str());
        }
        
        Gfx::Print(180, yPos + 100, 40, Gfx::COLOR_TEXT, descText, Gfx::ALIGN_VERTICAL);
        
        yPos += mCardHeight + mCardSpacing;
    }
    
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
        
        // 绘制警告图标和文字
        Gfx::DrawIcon(dialogX + dialogW/2, dialogY + 60, 80, Gfx::COLOR_WARNING, 0xf071, Gfx::ALIGN_HORIZONTAL);
        Gfx::Print(dialogX + dialogW/2, dialogY + 150, 50, Gfx::COLOR_TEXT, lang.Get("dialog.confirm.delete.titles"), Gfx::ALIGN_HORIZONTAL);
        Gfx::Print(dialogX + dialogW/2, dialogY + 215, 40, Gfx::COLOR_ALT_TEXT, lang.Get("dialog.warning.irreversible"), Gfx::ALIGN_HORIZONTAL);
        
        std::string confirmBtn = std::string("\ue000 ") + lang.Get("button.confirm");
        std::string cancelBtn = std::string("\ue001 ") + lang.Get("button.cancel");
        DrawBottomBar(confirmBtn.c_str(), nullptr, cancelBtn.c_str());
    } else {
        std::string bottomLeft = std::string("\ue07d ") + lang.Get("button.select") + 
                                 "  \ue000 " + lang.Get("button.confirm") + 
                                 "  \ue001 " + lang.Get("button.back");
        DrawBottomBar(bottomLeft.c_str(), nullptr, nullptr);
    }
}

bool SettingsScreen::Update(Input &input) {
    // 如果有子屏幕，更新子屏幕
    if (mSubscreen) {
        if (!mSubscreen->Update(input)) {
            mSubscreen.reset();  // 返回设置页面
        }
        return true;
    }
    
    // 如果显示确认对话框,只处理确认/取消
    if (mShowClearConfirm) {
        if (input.data.buttons_d & Input::BUTTON_A) {
            // 确认删除 SD 卡上的 title 文件夹
            const char* titleDir = "fs:/vol/external01/title";
            
            // 递归删除函数
            std::function<bool(const char*)> removeDirectory = [&](const char* path) -> bool {
                DIR* dir = opendir(path);
                if (!dir) {
                    return false;
                }
                
                struct dirent* entry;
                bool success = true;
                
                while ((entry = readdir(dir)) != nullptr) {
                    std::string name = entry->d_name;
                    if (name == "." || name == "..") {
                        continue;
                    }
                    
                    std::string fullPath = std::string(path) + "/" + name;
                    
                    struct stat st;
                    if (stat(fullPath.c_str(), &st) == 0) {
                        if (S_ISDIR(st.st_mode)) {
                            // 递归删除子目录
                            if (!removeDirectory(fullPath.c_str())) {
                                success = false;
                            }
                        } else {
                            // 删除文件
                            if (remove(fullPath.c_str()) != 0) {
                                WHBLogPrintf("Failed to delete file: %s", fullPath.c_str());
                                success = false;
                            }
                        }
                    }
                }
                
                closedir(dir);
                
                // 删除目录本身
                if (rmdir(path) != 0) {
                    WHBLogPrintf("Failed to remove directory: %s", path);
                    return false;
                }
                
                return success;
            };
            
            // 执行删除
            if (removeDirectory(titleDir)) {
                WHBLogPrintf("Successfully deleted title directory");
            } else {
                WHBLogPrintf("Failed to delete title directory (may not exist)");
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
    
    if (input.data.buttons_d & Input::BUTTON_UP) {
        if (mSelectedOption > SETTINGS_OPTION_MIN) {
            mSelectedOption = (SettingsOption)(mSelectedOption - 1);
        } else {
            mSelectedOption = SETTINGS_OPTION_MAX;
        }
    } else if (input.data.buttons_d & Input::BUTTON_DOWN) {
        if (mSelectedOption < SETTINGS_OPTION_MAX) {
            mSelectedOption = (SettingsOption)(mSelectedOption + 1);
        } else {
            mSelectedOption = SETTINGS_OPTION_MIN;
        }
    } else if (input.data.buttons_d & Input::BUTTON_A) {
        switch (mSelectedOption) {
            case SETTINGS_OPTION_LANGUAGE: {
                // 切换语言 (0=中文, 1=English, 2=日本語)
                int current = Config::GetInstance().GetLanguage();
                current = (current + 1) % 3;  // 循环切换 0-2
                Config::GetInstance().SetLanguage(current);
                Language::GetInstance().SetLanguage(static_cast<LanguageCode>(current));
                break;
            }
            case SETTINGS_OPTION_SHOW_COPY_DETECT: {
                // 切换文件检测提醒
                bool current = Config::GetInstance().ShowCopyFileDetection();
                Config::GetInstance().SetShowCopyFileDetection(!current);
                break;
            }
            case SETTINGS_OPTION_DOWNLOAD: {
                // 跳转到下载页面
                mSubscreen = std::make_unique<DownloadScreen>();
                break;
            }
            case SETTINGS_OPTION_CLEAR_TITLES: {
                // 显示确认对话框
                mShowClearConfirm = true;
                break;
            }
            case SETTINGS_OPTION_BACK: {
                return false; // 返回菜单
            }
        }
    } else if (input.data.buttons_d & Input::BUTTON_B) {
        return false; // 返回菜单
    }
    
    return true;
}
