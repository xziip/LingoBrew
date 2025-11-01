#include "MainScreen.hpp"
#include "Gfx.hpp"
#include "MenuScreen.hpp"
#include "common.h"
#include "utils/Language.hpp"
#include <mocha/mocha.h>
#include <utility>

MainScreen::~MainScreen() {
    if (mState > STATE_INIT_MOCHA) {
        Mocha_DeInitLibrary();
    }
}

void MainScreen::Draw() {
    Gfx::Clear(Gfx::COLOR_BACKGROUND);

    if (mMenuScreen) {
        mMenuScreen->Draw();
        return;
    }

    DrawTopBar(nullptr);
    
    Language& lang = Language::GetInstance();

    switch (mState) {
        case STATE_INIT:
            break;
        case STATE_INIT_MOCHA:
            if (mStateFailure) {
                DrawStatus(lang.Get("main.init.mocha.error"), Gfx::COLOR_ERROR);
                break;
            }

            DrawStatus(lang.Get("main.init.mocha.loading"));
            break;
        case STATE_INIT_FS:
            if (mStateFailure) {
                DrawStatus(lang.Get("main.init.fs.error"), Gfx::COLOR_ERROR);
                break;
            }
            DrawStatus(lang.Get("main.init.fs.loading"));
            break;
        case STATE_LOAD_MENU:
            DrawStatus(lang.Get("main.menu.loading"));
            break;
        case STATE_IN_MENU:
            break;
    }

    DrawBottomBar(mStateFailure ? nullptr : lang.Get("common.please_wait"), 
                  mStateFailure ? lang.Get("common.exit") : nullptr, nullptr);
}

bool MainScreen::Update(Input &input) {
    if (mMenuScreen) {
        return mMenuScreen->Update(input);
    }

    if (mStateFailure) {
        return true;
    }

    switch (mState) {
        case STATE_INIT:
            mState = STATE_INIT_MOCHA;
            break;
        case STATE_INIT_MOCHA: {
            MochaUtilsStatus status = Mocha_InitLibrary();
            if (status == MOCHA_RESULT_SUCCESS) {
                mState = STATE_INIT_FS;
                break;
            }

            mStateFailure = true;
            break;
        }
        case STATE_INIT_FS: {
            auto res = Mocha_MountFS(MLC_STORAGE_PATH, "/dev/mlc01", "/vol/storage_mlc01");
            if (res == MOCHA_RESULT_ALREADY_EXISTS) {
                res = Mocha_MountFS(MLC_STORAGE_PATH, nullptr, "/vol/storage_mlc01");
            }
            if (res != MOCHA_RESULT_SUCCESS) {
                mStateFailure = true;
                break;
            }
            
            // 挂载 SLCCMPT
            res = Mocha_MountFS(SLCCMPT_STORAGE_PATH, "/dev/slccmpt01", "/vol/storage_slccmpt01");
            if (res == MOCHA_RESULT_ALREADY_EXISTS) {
                res = Mocha_MountFS(SLCCMPT_STORAGE_PATH, nullptr, "/vol/storage_slccmpt01");
            }
            if (res != MOCHA_RESULT_SUCCESS) {
                mStateFailure = true;
                break;
            }
            
            // 挂载 SLC
            res = Mocha_MountFS(SLC_STORAGE_PATH, "/dev/slc01", "/vol/storage_slc01");
            if (res == MOCHA_RESULT_ALREADY_EXISTS) {
                res = Mocha_MountFS(SLC_STORAGE_PATH, nullptr, "/vol/storage_slc01");
            }
            if (res == MOCHA_RESULT_SUCCESS) {
                mState = STATE_LOAD_MENU;
                break;
            }

            mStateFailure = true;
            break;
        }
        case STATE_LOAD_MENU:
            mMenuScreen = std::make_unique<MenuScreen>();
            break;
        case STATE_IN_MENU:
            break;
    };

    return true;
}

void MainScreen::DrawStatus(std::string status, SDL_Color color) {
    Gfx::Print(Gfx::SCREEN_WIDTH / 2, Gfx::SCREEN_HEIGHT / 2, 64, color, std::move(status), Gfx::ALIGN_CENTER);
}
