#include "RestoreScreen.hpp"
#include "Gfx.hpp"
#include "RebootScreen.hpp"
#include "utils/Utils.hpp"
#include "utils/FileLogger.hpp"
#include "utils/Language.hpp"
#include <coreinit/filesystem.h>
#include <coreinit/launch.h>
#include <mocha/mocha.h>
#include <dirent.h>

RestoreScreen::RestoreScreen() 
    : mState(RESTORE_STATE_SELECT_STORAGE)
    , mStorageType(STORAGE_TYPE_MLC)
    , mRestoreTarget(RESTORE_TARGET_TITLE) {
}

RestoreScreen::~RestoreScreen() {
}

void RestoreScreen::DrawSimpleText(const std::vector<std::string> &items) {
    int yOff = mDefaultYPos;
    for (const auto &item : items) {
        Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, item, Gfx::ALIGN_VERTICAL);
        yOff += Gfx::GetTextHeight(mDefaultFontSize, item);
    }
}

void RestoreScreen::Draw() {
    Language& lang = Language::GetInstance();
    DrawTopBar(lang.Get("restore.title"));
    
    switch (mState) {
        case RESTORE_STATE_SELECT_STORAGE: {
            int yOff = mDefaultYPos;
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, lang.Get("restore.select_storage"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, lang.Get("restore.select_storage"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // MLC 选项
            SDL_Color mlcColor = (mStorageType == STORAGE_TYPE_MLC) ? Gfx::COLOR_WARNING : Gfx::COLOR_TEXT;
            std::string mlcText = (mStorageType == STORAGE_TYPE_MLC) ? "\ue805 " : "  ";
            mlcText += lang.Get("restore.mlc_storage");
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, mlcColor, mlcText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, mlcText);
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("restore.mlc_from"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("restore.mlc_from"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // SLC 选项
            SDL_Color slcColor = (mStorageType == STORAGE_TYPE_SLC) ? Gfx::COLOR_WARNING : Gfx::COLOR_TEXT;
            std::string slcText = (mStorageType == STORAGE_TYPE_SLC) ? "\ue805 " : "  ";
            slcText += lang.Get("restore.slc_storage");
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, slcColor, slcText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, slcText);
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("restore.slc_from"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("restore.slc_from"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // 绘制带发光效果的彩虹色警告
            SDL_Color glowColor = Gfx::GetRainbowColor(8.0f);
            std::string warningText = lang.Get("restore.warning");
            
            SDL_Color glowLayer = {glowColor.r, glowColor.g, glowColor.b, 80};
            Gfx::Print(mDefaultXPos - 2, yOff, mDefaultFontSize, glowLayer, warningText, Gfx::ALIGN_VERTICAL);
            Gfx::Print(mDefaultXPos + 2, yOff, mDefaultFontSize, glowLayer, warningText, Gfx::ALIGN_VERTICAL);
            Gfx::Print(mDefaultXPos, yOff - 2, mDefaultFontSize, glowLayer, warningText, Gfx::ALIGN_VERTICAL);
            Gfx::Print(mDefaultXPos, yOff + 2, mDefaultFontSize, glowLayer, warningText, Gfx::ALIGN_VERTICAL);
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, glowColor, warningText, Gfx::ALIGN_VERTICAL);
            
            break;
        }
        case RESTORE_STATE_SELECT_SLC_TYPE: {
            Language& lang = Language::GetInstance();
            int yOff = mDefaultYPos;
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, lang.Get("restore.select_slc_type"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, lang.Get("restore.select_slc_type"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // SLCCMPT 选项 (兼容模式 - 推荐)
            SDL_Color slccmptColor = (mSlcType == SLC_TYPE_SLCCMPT) ? Gfx::COLOR_WARNING : Gfx::COLOR_TEXT;
            std::string slccmptText = (mSlcType == SLC_TYPE_SLCCMPT) ? "\ue805 " : "  ";
            slccmptText += lang.Get("restore.slccmpt");
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, slccmptColor, slccmptText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, slccmptText);
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("restore.slccmpt_desc"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("restore.slccmpt_desc"));
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("restore.slccmpt_from"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("restore.slccmpt_from"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // SLC 选项 (直接访问 - 需谨慎)
            SDL_Color slcColor = (mSlcType == SLC_TYPE_SLC) ? Gfx::COLOR_WARNING : Gfx::COLOR_TEXT;
            std::string slcText = (mSlcType == SLC_TYPE_SLC) ? "\ue805 " : "  ";
            slcText += lang.Get("restore.slc_direct");
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, slcColor, slcText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, slcText);
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("restore.slc_direct_desc"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("restore.slc_direct_desc"));
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("restore.slc_from"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("restore.slc_from"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            break;
        }
        case RESTORE_STATE_ASK_TARGET: {
            // 绘制文本，选中项用黄色
            Language& lang = Language::GetInstance();
            int yOff = mDefaultYPos;
            
            if (mStorageType == STORAGE_TYPE_SLC) {
                // SLC 恢复只有一个选项
                std::string slcTitle = (mSlcType == SLC_TYPE_SLCCMPT) ? lang.Get("restore.slccmpt_title") : lang.Get("restore.slc_title");
                Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, slcTitle, Gfx::ALIGN_VERTICAL);
                yOff += Gfx::GetTextHeight(mDefaultFontSize, slcTitle);
                yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
                
                SDL_Color fullColor = Gfx::COLOR_WARNING;
                std::string fullText = (mSlcType == SLC_TYPE_SLCCMPT) ? std::string("\ue805 ") + lang.Get("restore.slccmpt_full") : std::string("\ue805 ") + lang.Get("restore.slc_full");
                Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, fullColor, fullText, Gfx::ALIGN_VERTICAL);
                yOff += Gfx::GetTextHeight(mDefaultFontSize, fullText);
                
                std::string slcPath = (mSlcType == SLC_TYPE_SLCCMPT) ? 
                    std::string("  ") + lang.Get("restore.slccmpt_path") : 
                    std::string("  ") + lang.Get("restore.slc_path");
                Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, slcPath, Gfx::ALIGN_VERTICAL);
                yOff += Gfx::GetTextHeight(mDefaultFontSize, slcPath);
                yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            } else {
                // MLC 恢复选项
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, lang.Get("restore.select_content"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, lang.Get("restore.select_content"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // Title 文件夹选项
            SDL_Color titleColor = (mRestoreTarget == RESTORE_TARGET_TITLE) ? Gfx::COLOR_WARNING : Gfx::COLOR_TEXT;
            std::string titleText = (mRestoreTarget == RESTORE_TARGET_TITLE) ? "\ue805 " : "  ";
            titleText += lang.Get("restore.title_folder");
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, titleColor, titleText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, titleText);
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("restore.title_path"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("restore.title_path"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // 字体文件选项
            SDL_Color pluginColor = (mRestoreTarget == RESTORE_TARGET_PLUGIN) ? Gfx::COLOR_WARNING : Gfx::COLOR_TEXT;
            std::string pluginText = (mRestoreTarget == RESTORE_TARGET_PLUGIN) ? "\ue805 " : "  ";
            pluginText += lang.Get("restore.fonts");
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, pluginColor, pluginText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, pluginText);
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("restore.fonts_path"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("restore.fonts_path"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            }
            
            // 绘制带发光效果的彩虹色警告（减弱）
            SDL_Color glowColor = Gfx::GetRainbowColor(8.0f);
            std::string warningText = lang.Get("restore.warning");
            
            // 只保留一层较弱的光晕
            SDL_Color glowLayer = {glowColor.r, glowColor.g, glowColor.b, 80};
            
            // 绘制光晕（4个主方向）
            Gfx::Print(mDefaultXPos - 2, yOff, mDefaultFontSize, glowLayer, warningText, Gfx::ALIGN_VERTICAL);
            Gfx::Print(mDefaultXPos + 2, yOff, mDefaultFontSize, glowLayer, warningText, Gfx::ALIGN_VERTICAL);
            Gfx::Print(mDefaultXPos, yOff - 2, mDefaultFontSize, glowLayer, warningText, Gfx::ALIGN_VERTICAL);
            Gfx::Print(mDefaultXPos, yOff + 2, mDefaultFontSize, glowLayer, warningText, Gfx::ALIGN_VERTICAL);
            
            // 绘制主文字
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, glowColor, warningText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, warningText);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // 绘制提示
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string(lang.Get("common.continue_hint")) + "  " + lang.Get("common.back_hint"), Gfx::ALIGN_VERTICAL);
            break;
        }
        case RESTORE_STATE_DO_RESTORE: {
            Language& lang = Language::GetInstance();
            std::vector<std::string> status;
            
            if (mBackupManager.IsScanning()) {
                // 正在扫描文件
                int scannedDirs = mBackupManager.GetScannedDirs();
                int foundFiles = mBackupManager.GetTotalItems();
                status.push_back(lang.Get("restore.scanning_backup"));
                status.push_back(Utils::sprintf(lang.Get("restore.scanned"), scannedDirs));
                status.push_back(Utils::sprintf(lang.Get("restore.found_files"), foundFiles));
                status.push_back(lang.Get("common.please_wait"));
            } else {
                // 正在恢复文件
                int total = mBackupManager.GetTotalItems();
                int processed = mBackupManager.GetProcessedItems();
                std::string currentFile = mBackupManager.GetCurrentFile();
                
                if (total > 0) {
                    int percent = (processed * 100) / total;
                    status.push_back(Utils::sprintf(lang.Get("restore.progress"), percent, processed, total));
                }

                if (!currentFile.empty()) {
                    std::string displayPath = currentFile;
                    
                    // 如果路径太长，只显示文件名
                    size_t lastSlash = displayPath.find_last_of("/\\");
                    if (lastSlash != std::string::npos && displayPath.length() > 40) {
                        displayPath = displayPath.substr(lastSlash + 1);
                    }

                    status.push_back(lang.Get("restore.restoring_file") + displayPath);
                    status.push_back(lang.Get("restore.wait_message"));
                }
            }

            DrawSimpleText(status);
            break;
        }
        case RESTORE_STATE_DONE:
        case RESTORE_STATE_ASK_REBOOT: {
            Language& lang = Language::GetInstance();
            std::vector<std::string> rebootText = {
                lang.Get("restore.complete"),
                "",
                mRestoreTarget == RESTORE_TARGET_TITLE ? 
                    lang.Get("restore.title_restored") :
                    lang.Get("restore.fonts_restored"),
                "",
                lang.Get("restore.ask_reboot"),
                "",
                lang.Get("restore.reboot_hint")
            };
            DrawSimpleText(rebootText);
            break;
        }
        case RESTORE_STATE_ERROR:
            {
                Language& lang = Language::GetInstance();
                DrawSimpleText({mError, "", lang.Get("common.back_hint")});
            }
            break;
    }

    // 根据状态显示不同的底部提示
    Language& lang2 = Language::GetInstance();
    std::string exitStr = lang2.Get("common.exit");
    std::string selectConfirm = std::string(lang2.Get("common.select")) + "  " + lang2.Get("common.confirm") + "  " + lang2.Get("common.back");
    std::string selectContinue = std::string(lang2.Get("common.select")) + "  " + lang2.Get("common.continue") + "  " + lang2.Get("common.back");
    std::string startRestore = std::string(lang2.Get("common.start_restore")) + "  " + lang2.Get("common.back");
    std::string inProgress = lang2.Get("restore.in_progress");
    std::string rebootReturn = std::string(lang2.Get("restore.reboot_hint")) + "  " + lang2.Get(" ");
    std::string backStr = lang2.Get("common.back");
    
    switch (mState) {
        case RESTORE_STATE_SELECT_STORAGE:
            DrawBottomBar(selectConfirm.c_str(), exitStr.c_str(), nullptr);
            break;
        case RESTORE_STATE_SELECT_SLC_TYPE:
            DrawBottomBar(selectContinue.c_str(), exitStr.c_str(), nullptr);
            break;
        case RESTORE_STATE_ASK_TARGET:
            if (mStorageType == STORAGE_TYPE_SLC) {
                DrawBottomBar(startRestore.c_str(), exitStr.c_str(), nullptr);
            } else {
                DrawBottomBar(selectConfirm.c_str(), exitStr.c_str(), nullptr);
            }
            break;
        case RESTORE_STATE_DO_RESTORE:
            DrawBottomBar(inProgress.c_str(), exitStr.c_str(), nullptr);
            break;
        case RESTORE_STATE_DONE:
        case RESTORE_STATE_ASK_REBOOT:
            DrawBottomBar(rebootReturn.c_str(), exitStr.c_str(), nullptr);
            break;
        case RESTORE_STATE_ERROR:
            DrawBottomBar(nullptr, exitStr.c_str(), backStr.c_str());
            break;
    }
}

bool RestoreScreen::Update(Input &input) {
    if (input.data.buttons_d & Input::BUTTON_B) {
        switch (mState) {
            case RESTORE_STATE_SELECT_STORAGE:
                return false;  // 返回菜单
            case RESTORE_STATE_SELECT_SLC_TYPE:
                mState = RESTORE_STATE_SELECT_STORAGE;  // 返回存储类型选择
                break;
            case RESTORE_STATE_ASK_TARGET:
                if (mStorageType == STORAGE_TYPE_SLC) {
                    mState = RESTORE_STATE_SELECT_SLC_TYPE;  // 返回 SLC 类型选择
                } else {
                    mState = RESTORE_STATE_SELECT_STORAGE;  // 返回存储类型选择
                }
                break;
            case RESTORE_STATE_DONE:
            case RESTORE_STATE_ASK_REBOOT:
                return false;  // 返回菜单
            case RESTORE_STATE_ERROR:
                return false;
            default:
                break;
        }
    }

    if (input.data.buttons_d & Input::BUTTON_A) {
        switch (mState) {
            case RESTORE_STATE_SELECT_STORAGE: {
                // 根据存储类型选择下一步
                if (mStorageType == STORAGE_TYPE_SLC) {
                    mState = RESTORE_STATE_SELECT_SLC_TYPE;
                } else {
                    mState = RESTORE_STATE_ASK_TARGET;
                }
                break;
            }
            case RESTORE_STATE_SELECT_SLC_TYPE: {
                // 进入选择恢复内容
                mState = RESTORE_STATE_ASK_TARGET;
                break;
            }
            case RESTORE_STATE_ASK_TARGET: {
                // 开始恢复
                std::string backupPath, destPath;
                const char* logPrefix = nullptr;
                
                if (mStorageType == STORAGE_TYPE_SLC) {
                    // SLC 恢复 - 根据类型选择路径
                    backupPath = (mSlcType == SLC_TYPE_SLCCMPT) ? BACKUP_SLCCMPT_PATH : BACKUP_SLC_PATH;
                    destPath = (mSlcType == SLC_TYPE_SLCCMPT) ? SLCCMPT_STORAGE_PATH ":/" : SLC_STORAGE_PATH ":/";
                    logPrefix = (mSlcType == SLC_TYPE_SLCCMPT) ? "slccmptrestore" : "slcrestore";
                } else if (mRestoreTarget == RESTORE_TARGET_TITLE) {
                    backupPath = BACKUP_TITLE_PATH;
                    destPath = HAX_DESTINATION_PATH;
                    logPrefix = "titlerestore";
                } else {
                    backupPath = BACKUP_FONTS_PATH;
                    destPath = HAX_PLUGINS_DESTINATION_PATH;
                    logPrefix = "fontsrestore";
                }
                
                // 检查备份是否存在
                DIR* dir = opendir(backupPath.c_str());
                if (!dir) {
                    mError = "错误：备份文件夹不存在";
                    mState = RESTORE_STATE_ERROR;
                    break;
                }
                closedir(dir);
                
                // 开始日志记录
                FileLogger::GetInstance().StartLog(logPrefix);
                FileLogger::GetInstance().Log("恢复开始");
                FileLogger::GetInstance().Log("源路径(备份): %s", backupPath.c_str());
                FileLogger::GetInstance().Log("目标路径(系统): %s", destPath.c_str());
                
                if (mBackupManager.StartBackup(backupPath, destPath)) {
                    mState = RESTORE_STATE_DO_RESTORE;
                } else {
                    FileLogger::GetInstance().LogError("无法开始恢复");
                    FileLogger::GetInstance().EndLog();
                    mError = "错误：无法开始恢复";
                    mState = RESTORE_STATE_ERROR;
                }
                break;
            }
            case RESTORE_STATE_DONE:
            case RESTORE_STATE_ASK_REBOOT: {
                // 重启
                OSLaunchTitlel(OS_TITLE_ID_REBOOT, 0);
                return true;
            }
            default:
                break;
        }
    }

    if (mState == RESTORE_STATE_SELECT_STORAGE) {
        if (input.data.buttons_d & Input::BUTTON_UP) {
            mStorageType = STORAGE_TYPE_MLC;
        }
        if (input.data.buttons_d & Input::BUTTON_DOWN) {
            mStorageType = STORAGE_TYPE_SLC;
        }
    }

    if (mState == RESTORE_STATE_SELECT_SLC_TYPE) {
        if (input.data.buttons_d & Input::BUTTON_UP) {
            mSlcType = SLC_TYPE_SLCCMPT;
        }
        if (input.data.buttons_d & Input::BUTTON_DOWN) {
            mSlcType = SLC_TYPE_SLC;
        }
    }

    if (mState == RESTORE_STATE_ASK_TARGET && mStorageType == STORAGE_TYPE_MLC) {
        if (input.data.buttons_d & Input::BUTTON_UP) {
            mRestoreTarget = RESTORE_TARGET_TITLE;
        }
        if (input.data.buttons_d & Input::BUTTON_DOWN) {
            mRestoreTarget = RESTORE_TARGET_PLUGIN;
        }
    }

    if (mState == RESTORE_STATE_DO_RESTORE) {
        bool inProgress = mBackupManager.UpdateBackup();
        if (!inProgress) {
            // 记录恢复完成日志
            int total = mBackupManager.GetTotalItems();
            int skipped = mBackupManager.GetSkippedItems();
            FileLogger::GetInstance().LogBackupEnd(total, skipped);
            FileLogger::GetInstance().LogSuccess("恢复成功完成");
            FileLogger::GetInstance().EndLog();
            
            mState = RESTORE_STATE_DONE;  // 直接进入完成状态，显示重启询问
        }
    }

    return true;
}
