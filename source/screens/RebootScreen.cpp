#include "RebootScreen.hpp"
#include "Gfx.hpp"
#include "../utils/Language.hpp"
#include <coreinit/launch.h>   // OSLaunchTitle, OS_TITLE_ID_REBOOT

RebootScreen::RebootScreen() {}

RebootScreen::~RebootScreen() = default;

void RebootScreen::Draw() {
    Language& lang = Language::GetInstance();
    DrawTopBar(lang.Get("menu.reboot"));

    std::vector<std::string> text = {
        lang.Get("reboot.confirm"),
        "",
        std::string("\ue000 ") + lang.Get("reboot.confirm.button"),
        std::string("\ue001 ") + lang.Get("button.cancel")
    };

    // 居中显示文本
    int yOff = (Gfx::SCREEN_HEIGHT - (text.size() * 60)) / 2;
    for (const auto &line : text) {
        Gfx::Print(Gfx::SCREEN_WIDTH / 2, yOff, 56, Gfx::COLOR_TEXT, line, Gfx::ALIGN_HORIZONTAL);
        yOff += 70;
    }

    std::string exitBtn = std::string("\ue044 ") + lang.Get("button.exit");
    std::string actionBtn = std::string("\ue000 ") + lang.Get("reboot.action") + " / \ue001 " + lang.Get("button.cancel");
    DrawBottomBar(nullptr, exitBtn.c_str(), actionBtn.c_str());
}

bool RebootScreen::Update(Input &input) {
    if (input.data.buttons_d & Input::BUTTON_A) {
        // 重启系统
        OSLaunchTitlel(OS_TITLE_ID_REBOOT, 0);
        return true; // 保持在当前界面，等待重启
    }
    
    if (input.data.buttons_d & Input::BUTTON_B) {
        // 取消，返回菜单
        return false;
    }

    return true;
}
