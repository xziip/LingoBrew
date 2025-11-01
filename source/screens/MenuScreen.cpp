#include "MenuScreen.hpp"
#include "AboutScreen.hpp"
#include "BackupScreen.hpp"
#include "CopyScreen.hpp"
#include "DebugScreen.hpp"
#include "DownloadScreen.hpp"
#include "RebootScreen.hpp"
#include "RestoreScreen.hpp"
#include "SettingsScreen.hpp"
#include "Gfx.hpp"
#include "../utils/Config.hpp"
#include "../utils/Language.hpp"

MenuScreen::MenuScreen()
    : mEntries({
              {MENU_ID_DUMP_LOGS, {0xf0C5, ""}},
              {MENU_ID_BACKUP, {0xf187, ""}},
              {MENU_ID_RESTORE, {0xf1da, ""}},
              {MENU_ID_REBOOT, {0xf021, ""}},
              {MENU_ID_SETTINGS, {0xf013, ""}},
              {MENU_ID_DEBUG, {0xf188, ""}},
              {MENU_ID_ABOUT, {0xf05a, ""}},
      }) {
    // 根据配置自动打开默认页面
    int defaultPage = Config::GetInstance().GetDefaultStartPage();
    if (defaultPage > 0 && defaultPage <= 8) {
        // 1=DUMP_LOGS, 2=BACKUP, 3=RESTORE, 4=REBOOT, 5=SETTINGS, 6=DEBUG, 7=ABOUT, 8=DOWNLOAD
        MenuID pageId = static_cast<MenuID>(defaultPage - 1);
        switch (pageId) {
            case MENU_ID_DUMP_LOGS:
                mSubscreen = std::make_unique<CopyScreen>();
                break;
            case MENU_ID_BACKUP:
                mSubscreen = std::make_unique<BackupScreen>();
                break;
            case MENU_ID_RESTORE:
                mSubscreen = std::make_unique<RestoreScreen>();
                break;
            case MENU_ID_REBOOT:
                mSubscreen = std::make_unique<RebootScreen>();
                break;
            case MENU_ID_SETTINGS:
                mSubscreen = std::make_unique<SettingsScreen>();
                break;
            case MENU_ID_DEBUG:
                mSubscreen = std::make_unique<DebugScreen>();
                break;
            case MENU_ID_ABOUT:
                mSubscreen = std::make_unique<AboutScreen>();
                break;
            case MENU_ID_DOWNLOAD:  // 对应页面 8 = 下载页面
                mSubscreen = std::make_unique<DownloadScreen>();
                break;
            default:
                break;
        }
    }
}

MenuScreen::~MenuScreen() = default;

MenuScreen::MenuID MenuScreen::GetMaxMenuID() const {
    // 下载页面不在主菜单中显示,最大ID是ABOUT
    return MENU_ID_ABOUT;
}

void MenuScreen::UpdateMenuRange() {
    MenuScreen::MenuID maxID = GetMaxMenuID();
    if (mSelectedEntry > maxID) {
        mSelectedEntry = maxID;
    }
}

void MenuScreen::Draw() {
    if (mSubscreen) {
        mSubscreen->Draw();
        return;
    }

    DrawTopBar(nullptr);

    // draw entries
    Language& lang = Language::GetInstance();
    MenuScreen::MenuID maxID = GetMaxMenuID();
    int displayIndex = 0;  // 实际显示的菜单索引
    for (MenuScreen::MenuID id = MENU_ID_MIN; id <= maxID;
         id        = static_cast<MenuScreen::MenuID>(id + 1)) {
        // 跳过 Debug 菜单如果它被隐藏
        if (id == MENU_ID_DEBUG && !Config::GetInstance().IsDebugMenuVisible()) {
            continue;
        }
        
        // 跳过下载页面(它不在主菜单中显示)
        if (id == MENU_ID_DOWNLOAD) {
            continue;
        }
        
        int yOff = 75 + displayIndex * 150;  // 使用 displayIndex 而不是 id
        Gfx::DrawRectFilled(0, yOff, Gfx::SCREEN_WIDTH, 150, Gfx::COLOR_ALT_BACKGROUND);
        Gfx::DrawIcon(68, yOff + 150 / 2, 60, Gfx::COLOR_TEXT, mEntries[id].icon);
        
        // 根据菜单ID获取对应的翻译文本
        const char* menuText = "";
        switch (id) {
            case MENU_ID_DUMP_LOGS: menuText = lang.Get("menu.copy"); break;
            case MENU_ID_BACKUP: menuText = lang.Get("menu.backup"); break;
            case MENU_ID_RESTORE: menuText = lang.Get("menu.restore"); break;
            case MENU_ID_REBOOT: menuText = lang.Get("menu.reboot"); break;
            case MENU_ID_SETTINGS: menuText = lang.Get("menu.settings"); break;
            case MENU_ID_DEBUG: menuText = lang.Get("menu.debug"); break;
            case MENU_ID_ABOUT: menuText = lang.Get("menu.about"); break;
            case MENU_ID_DOWNLOAD: break;  // 不在主菜单显示
        }
        
        Gfx::Print(128 + 8, yOff + 150 / 2, 60, Gfx::COLOR_TEXT, menuText, Gfx::ALIGN_VERTICAL);

        if (id == mSelectedEntry) {
            Gfx::DrawRect(0, yOff, Gfx::SCREEN_WIDTH, 150, 8, Gfx::COLOR_HIGHLIGHTED);
        }
        
        displayIndex++;  // 增加显示索引
    }

    DrawBottomBar(
        (std::string("\ue07d ") + lang.Get("button.navigate")).c_str(), 
        (std::string("\ue044 ") + lang.Get("button.exit")).c_str(), 
        (std::string("\ue000 ") + lang.Get("button.select")).c_str()
    );
}

bool MenuScreen::Update(Input &input) {
    if (mSubscreen) {
        if (!mSubscreen->Update(input)) {
            // subscreen wants to exit
            mSubscreen.reset();
            // 更新菜单范围(可能 Debug 菜单被隐藏了)
            UpdateMenuRange();
        }
        return true;
    }

    // 检测组合键: + 和 - 和 L 同时按住 (使用 buttons_h 检测按住状态)
    const uint32_t COMBO_MASK = Input::BUTTON_PLUS | Input::BUTTON_MINUS | Input::BUTTON_L;
    if ((input.data.buttons_h & COMBO_MASK) == COMBO_MASK) {
        // 三个键都按住时,显示 Debug 菜单
        if (!Config::GetInstance().IsDebugMenuVisible()) {
            Config::GetInstance().SetDebugMenuVisible(true);
            UpdateMenuRange();
        }
        return true;
    }

    MenuID maxID = GetMaxMenuID();
    
    if (input.data.buttons_d & Input::BUTTON_DOWN) {
        do {
            if (mSelectedEntry < maxID) {
                mSelectedEntry = static_cast<MenuID>(mSelectedEntry + 1);
            } else {
                break;
            }
            // 跳过隐藏的 Debug 菜单
        } while (mSelectedEntry == MENU_ID_DEBUG && !Config::GetInstance().IsDebugMenuVisible());
    } else if (input.data.buttons_d & Input::BUTTON_UP) {
        do {
            if (mSelectedEntry > MENU_ID_MIN) {
                mSelectedEntry = static_cast<MenuID>(mSelectedEntry - 1);
            } else {
                break;
            }
            // 跳过隐藏的 Debug 菜单
        } while (mSelectedEntry == MENU_ID_DEBUG && !Config::GetInstance().IsDebugMenuVisible());
    }

    if (input.data.buttons_d & Input::BUTTON_A) {
        switch (mSelectedEntry) {
            case MENU_ID_DUMP_LOGS:
                mSubscreen = std::make_unique<CopyScreen>();
                break;
            case MENU_ID_BACKUP:
                mSubscreen = std::make_unique<BackupScreen>();
                break;
            case MENU_ID_RESTORE:
                mSubscreen = std::make_unique<RestoreScreen>();
                break;
            case MENU_ID_REBOOT:
                mSubscreen = std::make_unique<RebootScreen>();
                break;
            case MENU_ID_SETTINGS:
                mSubscreen = std::make_unique<SettingsScreen>();
                break;
            case MENU_ID_DEBUG:
                mSubscreen = std::make_unique<DebugScreen>();
                break;
            case MENU_ID_ABOUT:
                mSubscreen = std::make_unique<AboutScreen>();
                break;
            case MENU_ID_DOWNLOAD:
                // 下载页面不在主菜单中，通过Debug页面的默认页面访问
                break;
        }
    }

    return true;
}
