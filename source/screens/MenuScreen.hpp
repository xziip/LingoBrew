#pragma once

#include "Screen.hpp"
#include <map>
#include <memory>

class MenuScreen : public Screen {
public:
    MenuScreen();

    ~MenuScreen() override;

    void Draw() override;

    bool Update(Input &input) override;

private:
    std::unique_ptr<Screen> mSubscreen;

    enum MenuID {
        MENU_ID_DUMP_LOGS,
        MENU_ID_BACKUP,
        MENU_ID_RESTORE,
        MENU_ID_REBOOT,
        MENU_ID_SETTINGS,
        MENU_ID_DEBUG,
        MENU_ID_ABOUT,
        MENU_ID_DOWNLOAD,  // 下载页面(仅用于debug默认页面)

        MENU_ID_MIN = MENU_ID_DUMP_LOGS,
        MENU_ID_MAX = MENU_ID_DOWNLOAD,
    };

    struct MenuEntry {
        uint16_t icon;
        const char *name;
    };
    std::map<MenuID, MenuEntry> mEntries;
    MenuID mSelectedEntry = MENU_ID_MIN;
    
    // Debug 菜单相关
    void UpdateMenuRange();
    MenuID GetMaxMenuID() const;
};
