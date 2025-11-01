#include "AboutScreen.hpp"
#include "../utils/Language.hpp"

AboutScreen::AboutScreen() {
    Language& lang = Language::GetInstance();
    
    creditList.emplace_back(lang.Get("about.developer"), "Xziip");
    creditList.emplace_back(" ", " ");
    creditList.emplace_back("", std::string("\n") + lang.Get("about.based_on"));

    fontList.emplace_back(lang.Get("about.main_font"), lang.Get("about.main_font.name"));
    fontList.emplace_back(lang.Get("about.icon_font"), "FontAwesome");
    fontList.emplace_back(lang.Get("about.mono_font"), "Terminus Font");

    linkList.emplace_back("", ScreenListElement{"https://github.com/xziip/lingobrew", true});
}

AboutScreen::~AboutScreen() = default;

void AboutScreen::Draw() {
    Language& lang = Language::GetInstance();
    DrawTopBar(lang.Get("menu.about"));

    int yOff = 128;
    yOff     = DrawHeader(32, yOff, 896, 0xf121, lang.Get("about.credits"));
    yOff     = DrawList(32, yOff, 896, creditList);
    yOff     = DrawHeader(32, yOff, 896, 0xf031, lang.Get("about.fonts"));
    yOff     = DrawList(32, yOff, 896, fontList);

    yOff = 128;
    yOff = DrawHeader(992, yOff, 896, 0xf08e, lang.Get("about.source"));
    yOff = DrawList(992, yOff, 896, linkList);

    std::string exitBtn = std::string("\ue044 ") + lang.Get("button.exit");
    std::string backBtn = std::string("\ue001 ") + lang.Get("button.back");
    DrawBottomBar(nullptr, exitBtn.c_str(), backBtn.c_str());
}

bool AboutScreen::Update(Input &input) {
    return !(input.data.buttons_d & Input::BUTTON_B);
}
