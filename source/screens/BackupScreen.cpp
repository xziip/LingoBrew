#include "BackupScreen.hpp"
#include "Gfx.hpp"
#include "common.h"
#include "utils/Utils.hpp"
#include "utils/FileLogger.hpp"
#include "utils/Language.hpp"
#include <ctime>

BackupScreen::BackupScreen() {
    // 设置错误回调
    mBackupManager.SetErrorCallback([this](const std::string& error) {
        mError = error;
        mBackupState = BACKUP_STATE_ERROR;
    });
}

BackupScreen::~BackupScreen() {
}

void BackupScreen::DrawSimpleText(const std::string &text) const {
    Gfx::Print(Gfx::SCREEN_WIDTH / 2, Gfx::SCREEN_HEIGHT / 2, mDefaultFontSize, Gfx::COLOR_TEXT, text, Gfx::ALIGN_CENTER);
}

void BackupScreen::DrawSimpleText(const std::vector<std::string> &items) const {
    int yOff = mDefaultYPos;
    for (const auto &item : items) {
        Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, item, Gfx::ALIGN_VERTICAL);
        yOff += Gfx::GetTextHeight(mDefaultFontSize, item);
    }
}

void BackupScreen::Draw() {
    Language& lang = Language::GetInstance();
    DrawTopBar(lang.Get("backup.title"));
    
    switch (mBackupState) {
        case BACKUP_STATE_SELECT_STORAGE: {
            int yOff = mDefaultYPos;
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, lang.Get("backup.select_storage"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, lang.Get("backup.select_storage"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // MLC 选项
            SDL_Color mlcColor = (mStorageType == STORAGE_TYPE_MLC) ? Gfx::COLOR_WARNING : Gfx::COLOR_TEXT;
            std::string mlcText = (mStorageType == STORAGE_TYPE_MLC) ? "\ue805 " : "  ";
            mlcText += lang.Get("backup.mlc_storage");
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, mlcColor, mlcText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, mlcText);
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.mlc_desc1"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.mlc_desc1"));
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.mlc_desc2"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.mlc_desc2"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // SLC 选项
            SDL_Color slcColor = (mStorageType == STORAGE_TYPE_SLC) ? Gfx::COLOR_WARNING : Gfx::COLOR_TEXT;
            std::string slcText = (mStorageType == STORAGE_TYPE_SLC) ? "\ue805 " : "  ";
            slcText += lang.Get("backup.slc_storage");
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, slcColor, slcText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, slcText);
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.slc_desc1"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.slc_desc1"));
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.slc_desc2"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.slc_desc2"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            break;
        }
        case BACKUP_STATE_SELECT: {
            int yOff = mDefaultYPos;
            
            std::string title = (mStorageType == STORAGE_TYPE_MLC) ? lang.Get("backup.select_mlc") : lang.Get("backup.select_slc");
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, title, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, title);
            
            if (mStorageType == STORAGE_TYPE_SLC) {
                // SLC 只有一个选项：完整备份
                SDL_Color fullColor = Gfx::COLOR_WARNING;
                std::string fullText = std::string("\ue805 ") + lang.Get("backup.full_slc");
                Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, fullColor, fullText, Gfx::ALIGN_VERTICAL);
                yOff += Gfx::GetTextHeight(mDefaultFontSize, fullText);
                
                Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.full_slc_desc"), Gfx::ALIGN_VERTICAL);
                yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.full_slc_desc"));
                
                Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.full_slc_path"), Gfx::ALIGN_VERTICAL);
                yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.full_slc_path"));
                yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            } else {
                // MLC 的原有选项
            
                // Title 文件夹选项
            SDL_Color titleColor = (mBackupTarget == BACKUP_TARGET_TITLE) ? Gfx::COLOR_WARNING : Gfx::COLOR_TEXT;
            std::string titleText = (mBackupTarget == BACKUP_TARGET_TITLE) ? "\ue805 " : "  ";
            titleText += lang.Get("backup.title_folder");
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, titleColor, titleText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, titleText);
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.title_path"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.title_path"));
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.title_dest"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.title_dest"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // 字体文件选项
            SDL_Color fontsColor = (mBackupTarget == BACKUP_TARGET_FONTS) ? Gfx::COLOR_WARNING : Gfx::COLOR_TEXT;
            std::string fontsText = (mBackupTarget == BACKUP_TARGET_FONTS) ? "\ue805 " : "  ";
            fontsText += lang.Get("backup.fonts");
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, fontsColor, fontsText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, fontsText);
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.fonts_path"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.fonts_path"));
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.fonts_dest"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.fonts_dest"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // 完整MLC备份选项
            SDL_Color fullColor = (mBackupTarget == BACKUP_TARGET_FULL) ? Gfx::COLOR_WARNING : Gfx::COLOR_TEXT;
            std::string fullText = (mBackupTarget == BACKUP_TARGET_FULL) ? "\ue805 " : "  ";
            fullText += lang.Get("backup.full_mlc");
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, fullColor, fullText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, fullText);
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.full_mlc_desc"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.full_mlc_desc"));
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.full_mlc_dest"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.full_mlc_dest"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            }
            
            break;
        }
        case BACKUP_STATE_SELECT_MODE: {
            Language& lang = Language::GetInstance();
            int yOff = mDefaultYPos;
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, lang.Get("backup.select_mode"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, lang.Get("backup.select_mode"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // 全量备份选项 - 只有这一行用黄色
            SDL_Color allColor = (mBackupMode == BACKUP_MODE_ALL) ? Gfx::COLOR_WARNING : Gfx::COLOR_TEXT;
            std::string allText = (mBackupMode == BACKUP_MODE_ALL) ? "\ue805 " : "  ";
            allText += lang.Get("backup.mode_full");
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, allColor, allText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, allText);
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  - ") + lang.Get("backup.mode_full_desc1"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  - ") + lang.Get("backup.mode_full_desc1"));
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  - ") + lang.Get("backup.mode_full_desc2"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  - ") + lang.Get("backup.mode_full_desc2"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // 选择性备份选项 - 只有这一行用黄色
            SDL_Color selectiveColor = (mBackupMode == BACKUP_MODE_SELECTIVE) ? Gfx::COLOR_WARNING : Gfx::COLOR_TEXT;
            std::string selectiveText = (mBackupMode == BACKUP_MODE_SELECTIVE) ? "\ue805 " : "  ";
            selectiveText += lang.Get("backup.mode_selective");
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, selectiveColor, selectiveText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, selectiveText);
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  - ") + lang.Get("backup.mode_selective_desc1"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  - ") + lang.Get("backup.mode_selective_desc1"));
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  - ") + lang.Get("backup.mode_selective_desc2"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  - ") + lang.Get("backup.mode_selective_desc2"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string(lang.Get("common.start_hint")) + "  " + lang.Get("common.back_hint"), Gfx::ALIGN_VERTICAL);
            break;
        }
        case BACKUP_STATE_SELECT_FULL: {
            Language& lang = Language::GetInstance();
            int yOff = mDefaultYPos;
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, lang.Get("backup.select_mlc_folder"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, lang.Get("backup.select_mlc_folder"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // sys 文件夹选项
            SDL_Color sysColor = (mFullBackupTarget == FULL_BACKUP_SYS) ? Gfx::COLOR_WARNING : Gfx::COLOR_TEXT;
            std::string sysText = (mFullBackupTarget == FULL_BACKUP_SYS) ? "\ue805 " : "  ";
            sysText += lang.Get("backup.sys_folder");
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, sysColor, sysText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, sysText);
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.sys_desc"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.sys_desc"));
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.sys_path"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.sys_path"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // usr 文件夹选项
            SDL_Color usrColor = (mFullBackupTarget == FULL_BACKUP_USR) ? Gfx::COLOR_WARNING : Gfx::COLOR_TEXT;
            std::string usrText = (mFullBackupTarget == FULL_BACKUP_USR) ? "\ue805 " : "  ";
            usrText += lang.Get("backup.usr_folder");
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, usrColor, usrText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, usrText);
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.usr_desc"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.usr_desc"));
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.usr_path"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.usr_path"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string(lang.Get("common.start_hint")) + "  " + lang.Get("common.back_hint"), Gfx::ALIGN_VERTICAL);
            break;
        }
        case BACKUP_STATE_SELECT_SLC_TYPE: {
            Language& lang = Language::GetInstance();
            int yOff = mDefaultYPos;
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, lang.Get("backup.select_slc_type"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, lang.Get("backup.select_slc_type"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // SLCCMPT 选项 (兼容模式 - 推荐)
            SDL_Color slccmptColor = (mSlcType == SLC_TYPE_SLCCMPT) ? Gfx::COLOR_WARNING : Gfx::COLOR_TEXT;
            std::string slccmptText = (mSlcType == SLC_TYPE_SLCCMPT) ? "\ue805 " : "  ";
            slccmptText += lang.Get("backup.slccmpt");
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, slccmptColor, slccmptText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, slccmptText);
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.slccmpt_desc"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.slccmpt_desc"));
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.slccmpt_path"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.slccmpt_path"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // SLC 选项 (直接访问 - 需谨慎)
            SDL_Color slcColor = (mSlcType == SLC_TYPE_SLC) ? Gfx::COLOR_WARNING : Gfx::COLOR_TEXT;
            std::string slcText = (mSlcType == SLC_TYPE_SLC) ? "\ue805 " : "  ";
            slcText += lang.Get("backup.slc_direct");
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, slcColor, slcText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, slcText);
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.slc_direct_desc"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.slc_direct_desc"));
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string("  ") + lang.Get("backup.slc_direct_path"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, std::string("  ") + lang.Get("backup.slc_direct_path"));
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, std::string(lang.Get("common.continue_hint")) + "  " + lang.Get("common.back_hint"), Gfx::ALIGN_VERTICAL);
            break;
        }
        case BACKUP_STATE_DO_BACKUP: {
            Language& lang = Language::GetInstance();
            std::vector<std::string> status;
            
            if (mBackupManager.IsScanning()) {
                // 正在扫描文件
                int scannedDirs = mBackupManager.GetScannedDirs();
                int pendingDirs = mBackupManager.GetPendingScanDirs();
                int foundItems = mBackupManager.GetTotalItems();
                status.push_back(lang.Get("backup.scanning"));
                status.push_back("");
                status.push_back(Utils::sprintf(lang.Get("backup.scanned_dirs"), scannedDirs));
                status.push_back(Utils::sprintf(lang.Get("backup.pending_dirs"), pendingDirs));
                status.push_back(Utils::sprintf(lang.Get("backup.found_items"), foundItems));
                status.push_back(std::string("  ") + lang.Get("backup.items_include"));
                status.push_back("");
                status.push_back(lang.Get("backup.wait_scanning"));
            } else {
                // 正在备份文件
                int total = mBackupManager.GetTotalItems();
                int processed = mBackupManager.GetProcessedItems();
                int skipped = mBackupManager.GetSkippedItems();
                std::string currentFile = mBackupManager.GetCurrentFile();
                
                if (total > 0) {
                    int percent = (processed * 100) / total;
                    status.push_back(Utils::sprintf(lang.Get("backup.progress"), percent, processed, total));
                    if (skipped > 0) {
                        status.push_back(Utils::sprintf(std::string("  ") + lang.Get("backup.skipped_files"), skipped));
                    }
                }

                if (!currentFile.empty()) {
                    std::string displayPath = currentFile;
                    
                    // 如果路径太长，只显示文件名
                    size_t lastSlash = displayPath.find_last_of("/\\");
                    if (lastSlash != std::string::npos && displayPath.length() > 50) {
                        displayPath = "..." + displayPath.substr(lastSlash);
                    }

                    status.push_back("");
                    status.push_back(lang.Get("backup.current_file") + displayPath);
                    
                    // 显示当前文件的复制进度
                    if (mBackupManager.IsCopyingFile()) {
                        long fileSize = mBackupManager.GetCurrentFileSize();
                        long fileCopied = mBackupManager.GetCurrentFileCopied();
                        
                        if (fileSize > 0) {
                            int filePercent = (int)((fileCopied * 100) / fileSize);
                            
                            // 格式化文件大小显示
                            std::string sizeStr;
                            if (fileSize >= 1024 * 1024) {
                                // MB
                                double sizeMB = fileSize / (1024.0 * 1024.0);
                                double copiedMB = fileCopied / (1024.0 * 1024.0);
                                sizeStr = Utils::sprintf("%.2f MB / %.2f MB", copiedMB, sizeMB);
                            } else if (fileSize >= 1024) {
                                // KB
                                double sizeKB = fileSize / 1024.0;
                                double copiedKB = fileCopied / 1024.0;
                                sizeStr = Utils::sprintf("%.2f KB / %.2f KB", copiedKB, sizeKB);
                            } else {
                                // Bytes
                                sizeStr = Utils::sprintf("%ld B / %ld B", fileCopied, fileSize);
                            }
                            
                            status.push_back(Utils::sprintf(lang.Get("backup.file_progress"), filePercent, sizeStr.c_str()));
                        }
                    }
                    
                    status.push_back("");
                    status.push_back(lang.Get("backup.wait_copying"));
                }
            }

            DrawSimpleText(status);
            break;
        }
        case BACKUP_STATE_DONE: {
            Language& lang = Language::GetInstance();
            std::vector<std::string> doneText;
            doneText.push_back(lang.Get("backup.complete"));
            doneText.push_back("");
            
            // 显示备份统计
            int totalItems = mBackupManager.GetTotalItems();
            int scannedDirs = mBackupManager.GetScannedDirs();
            int skippedItems = mBackupManager.GetSkippedItems();
            doneText.push_back(Utils::sprintf(lang.Get("backup.total_items"), totalItems));
            doneText.push_back(Utils::sprintf(lang.Get("backup.scanned_dirs"), scannedDirs));
            if (skippedItems > 0) {
                doneText.push_back(Utils::sprintf(lang.Get("backup.skipped_count"), skippedItems));
            }
            doneText.push_back("");
            
            if (mBackupTarget == BACKUP_TARGET_TITLE) {
                doneText.push_back(lang.Get("backup.title_backed_up"));
                doneText.push_back(lang.Get("backup.title_dest"));
            } else if (mBackupTarget == BACKUP_TARGET_FONTS) {
                doneText.push_back(lang.Get("backup.fonts_backed_up"));
                doneText.push_back(lang.Get("backup.fonts_dest"));
            } else if (mBackupTarget == BACKUP_TARGET_FULL) {
                if (mStorageType == STORAGE_TYPE_SLC) {
                    if (mSlcType == SLC_TYPE_SLCCMPT) {
                        doneText.push_back(lang.Get("backup.slccmpt_backed_up"));
                        doneText.push_back(lang.Get("backup.slccmpt_dest"));
                    } else {
                        doneText.push_back(lang.Get("backup.slc_backed_up"));
                        doneText.push_back(lang.Get("backup.slc_dest"));
                    }
                } else if (mFullBackupTarget == FULL_BACKUP_SYS) {
                    doneText.push_back(lang.Get("backup.sys_backed_up"));
                    doneText.push_back(lang.Get("backup.sys_dest"));
                } else {
                    doneText.push_back(lang.Get("backup.usr_backed_up"));
                    doneText.push_back(lang.Get("backup.usr_dest"));
                }
            }
            
            doneText.push_back("");
            doneText.push_back(lang.Get("common.return_menu"));
            
            DrawSimpleText(doneText);
            break;
        }
        case BACKUP_STATE_ERROR:
            {
                Language& lang = Language::GetInstance();
                DrawSimpleText(mError + ". " + lang.Get("common.back_hint"));
            }
            break;
    }

    // 根据状态显示不同的底部提示
    const char* leftHint = nullptr;
    std::string centerHintStr = lang.Get("common.exit");
    std::string rightHintStr = lang.Get("common.back_hint");
    const char* centerHint = centerHintStr.c_str();
    const char* rightHint = rightHintStr.c_str();
    
    std::string selectContinue = std::string(lang.Get("common.select")) + " / " + lang.Get("common.continue") + " / " + lang.Get("common.back");
    std::string selectStart = std::string(lang.Get("common.select")) + " / " + lang.Get("common.start") + " / " + lang.Get("common.back");
    std::string startBack = std::string(lang.Get("common.start")) + " / " + lang.Get("common.back");
    std::string returnMenu = lang.Get("common.return");
    
    if (mBackupState == BACKUP_STATE_SELECT_STORAGE) {
        rightHint = selectContinue.c_str();
    } else if (mBackupState == BACKUP_STATE_SELECT_SLC_TYPE) {
        rightHint = selectContinue.c_str();
    } else if (mBackupState == BACKUP_STATE_SELECT) {
        if (mStorageType == STORAGE_TYPE_SLC) {
            rightHint = startBack.c_str();
        } else {
            rightHint = selectContinue.c_str();
        }
    } else if (mBackupState == BACKUP_STATE_SELECT_MODE) {
        rightHint = selectStart.c_str();
    } else if (mBackupState == BACKUP_STATE_SELECT_FULL) {
        rightHint = selectStart.c_str();
    } else if (mBackupState == BACKUP_STATE_DONE) {
        rightHint = returnMenu.c_str();
    }
    
    DrawBottomBar(leftHint, centerHint, rightHint);
}

bool BackupScreen::Update(Input &input) {
    switch (mBackupState) {
        case BACKUP_STATE_SELECT_STORAGE: {
            if (input.data.buttons_d & Input::BUTTON_UP) {
                mStorageType = STORAGE_TYPE_MLC;
            } else if (input.data.buttons_d & Input::BUTTON_DOWN) {
                mStorageType = STORAGE_TYPE_SLC;
            } else if (input.data.buttons_d & Input::BUTTON_A) {
                // 根据存储类型进入不同状态
                if (mStorageType == STORAGE_TYPE_SLC) {
                    // SLC 需要先选择类型
                    mBackupState = BACKUP_STATE_SELECT_SLC_TYPE;
                } else {
                    // MLC 直接选择备份内容
                    mBackupState = BACKUP_STATE_SELECT;
                }
            } else if (input.data.buttons_d & Input::BUTTON_B) {
                // 返回菜单
                return false;
            }
            break;
        }
        case BACKUP_STATE_SELECT: {
            if (mStorageType == STORAGE_TYPE_SLC) {
                // SLC 只有一个选项,直接开始备份
                if (input.data.buttons_d & Input::BUTTON_A) {
                    mBackupState = BACKUP_STATE_DO_BACKUP;
                    mBackupTarget = BACKUP_TARGET_FULL;
                    
                    // 根据 SLC 类型选择路径
                    std::string sourcePath = (mSlcType == SLC_TYPE_SLCCMPT) ? 
                        SLCCMPT_STORAGE_PATH ":/" : SLC_STORAGE_PATH ":/";
                    std::string backupPath = (mSlcType == SLC_TYPE_SLCCMPT) ? 
                        BACKUP_SLCCMPT_PATH : BACKUP_SLC_PATH;
                    
                    // 开始日志记录
                    const char* logPrefix = (mSlcType == SLC_TYPE_SLCCMPT) ? "slccmptbackup" : "slcbackup";
                    FileLogger::GetInstance().StartLog(logPrefix);
                    FileLogger::GetInstance().LogBackupStart(sourcePath.c_str(), backupPath.c_str());
                    
                    bool success = mBackupManager.StartBackup(sourcePath, backupPath);
                    
                    if (!success) {
                        Language& lang = Language::GetInstance();
                        FileLogger::GetInstance().LogError("创建备份目录失败");
                        FileLogger::GetInstance().EndLog();
                        mError = lang.Get("backup.error_create_dir");
                        mBackupState = BACKUP_STATE_ERROR;
                    }
                } else if (input.data.buttons_d & Input::BUTTON_B) {
                    // 返回 SLC 类型选择
                    mBackupState = BACKUP_STATE_SELECT_SLC_TYPE;
                }
            } else {
                // MLC 的原有逻辑
                if (input.data.buttons_d & Input::BUTTON_UP) {
                    // 向上循环选择
                    if (mBackupTarget == BACKUP_TARGET_TITLE) {
                        mBackupTarget = BACKUP_TARGET_FULL;
                    } else if (mBackupTarget == BACKUP_TARGET_FONTS) {
                        mBackupTarget = BACKUP_TARGET_TITLE;
                    } else {
                        mBackupTarget = BACKUP_TARGET_FONTS;
                    }
                } else if (input.data.buttons_d & Input::BUTTON_DOWN) {
                    // 向下循环选择
                    if (mBackupTarget == BACKUP_TARGET_TITLE) {
                        mBackupTarget = BACKUP_TARGET_FONTS;
                    } else if (mBackupTarget == BACKUP_TARGET_FONTS) {
                        mBackupTarget = BACKUP_TARGET_FULL;
                    } else {
                        mBackupTarget = BACKUP_TARGET_TITLE;
                    }
                } else if (input.data.buttons_d & Input::BUTTON_A) {
                    // 根据选择的目标进入不同状态
                    if (mBackupTarget == BACKUP_TARGET_FULL) {
                        // 完整备份 - 进入选择 sys/usr 状态
                        mBackupState = BACKUP_STATE_SELECT_FULL;
                    } else {
                        // Title或Fonts - 进入备份模式选择
                        mBackupState = BACKUP_STATE_SELECT_MODE;
                    }
                } else if (input.data.buttons_d & Input::BUTTON_B) {
                    // 返回存储类型选择
                    mBackupState = BACKUP_STATE_SELECT_STORAGE;
                }
            }
            break;
        }
        case BACKUP_STATE_SELECT_MODE: {
            if (input.data.buttons_d & Input::BUTTON_UP) {
                mBackupMode = BACKUP_MODE_ALL;
            } else if (input.data.buttons_d & Input::BUTTON_DOWN) {
                mBackupMode = BACKUP_MODE_SELECTIVE;
            } else if (input.data.buttons_d & Input::BUTTON_A) {
                // 开始备份
                mBackupState = BACKUP_STATE_DO_BACKUP;
                
                std::string sourcePath, sdSourcePath, backupPath;
                const char* logPrefix = nullptr;
                
                if (mBackupTarget == BACKUP_TARGET_TITLE) {
                    sourcePath = HAX_DESTINATION_PATH;
                    sdSourcePath = HAX_SOURCE_PATH;
                    backupPath = BACKUP_TITLE_PATH;
                    logPrefix = "titlebackup";
                } else {
                    sourcePath = HAX_PLUGINS_DESTINATION_PATH;
                    sdSourcePath = HAX_PLUGINS_SOURCE_PATH;
                    backupPath = BACKUP_FONTS_PATH;
                    logPrefix = "fontsbackup";
                }
                
                // 开始日志记录
                FileLogger::GetInstance().StartLog(logPrefix);
                FileLogger::GetInstance().LogBackupStart(sourcePath.c_str(), backupPath.c_str());
                
                bool success;
                if (mBackupMode == BACKUP_MODE_ALL) {
                    FileLogger::GetInstance().Log("备份模式: 全量备份");
                    success = mBackupManager.StartBackup(sourcePath, backupPath);
                } else {
                    FileLogger::GetInstance().Log("备份模式: 选择性备份");
                    FileLogger::GetInstance().Log("SD源路径: %s", sdSourcePath.c_str());
                    success = mBackupManager.StartSelectiveBackup(sourcePath, sdSourcePath, backupPath);
                }
                
                if (!success) {
                    Language& lang = Language::GetInstance();
                    FileLogger::GetInstance().LogError("创建备份目录失败");
                    FileLogger::GetInstance().EndLog();
                    mError = lang.Get("backup.error_create_dir");
                    mBackupState = BACKUP_STATE_ERROR;
                }
            } else if (input.data.buttons_d & Input::BUTTON_B) {
                // 返回上一级
                mBackupState = BACKUP_STATE_SELECT;
            }
            break;
        }
        case BACKUP_STATE_SELECT_SLC_TYPE: {
            if (input.data.buttons_d & Input::BUTTON_UP) {
                mSlcType = SLC_TYPE_SLCCMPT;
            } else if (input.data.buttons_d & Input::BUTTON_DOWN) {
                mSlcType = SLC_TYPE_SLC;
            } else if (input.data.buttons_d & Input::BUTTON_A) {
                // 继续到备份内容选择
                mBackupState = BACKUP_STATE_SELECT;
            } else if (input.data.buttons_d & Input::BUTTON_B) {
                // 返回存储类型选择
                mBackupState = BACKUP_STATE_SELECT_STORAGE;
            }
            break;
        }
        case BACKUP_STATE_SELECT_FULL: {
            if (input.data.buttons_d & Input::BUTTON_UP) {
                mFullBackupTarget = FULL_BACKUP_SYS;
            } else if (input.data.buttons_d & Input::BUTTON_DOWN) {
                mFullBackupTarget = FULL_BACKUP_USR;
            } else if (input.data.buttons_d & Input::BUTTON_A) {
                // 开始完整备份
                mBackupState = BACKUP_STATE_DO_BACKUP;
                
                std::string sourcePath = (mFullBackupTarget == FULL_BACKUP_SYS) ? 
                    MLC_STORAGE_PATH ":/sys" : MLC_STORAGE_PATH ":/usr";
                std::string backupPath = (mFullBackupTarget == FULL_BACKUP_SYS) ? 
                    "fs:/vol/external01/mlcbackup/full/sys" : "fs:/vol/external01/mlcbackup/full/usr";
                
                // 开始日志记录
                const char* logPrefix = (mFullBackupTarget == FULL_BACKUP_SYS) ? "mlcsysbackup" : "mlcusrbackup";
                FileLogger::GetInstance().StartLog(logPrefix);
                FileLogger::GetInstance().LogBackupStart(sourcePath.c_str(), backupPath.c_str());
                
                // 完整备份始终使用全量模式
                bool success = mBackupManager.StartBackup(sourcePath, backupPath);
                
                if (!success) {
                    Language& lang = Language::GetInstance();
                    FileLogger::GetInstance().LogError("创建备份目录失败");
                    FileLogger::GetInstance().EndLog();
                    mError = lang.Get("backup.error_create_dir");
                    mBackupState = BACKUP_STATE_ERROR;
                }
            } else if (input.data.buttons_d & Input::BUTTON_B) {
                // 返回上一级
                mBackupState = BACKUP_STATE_SELECT;
            }
            break;
        }
        case BACKUP_STATE_DO_BACKUP: {
            // 更新备份进度
            if (mBackupManager.IsBackupInProgress()) {
                if (!mBackupManager.UpdateBackup()) {
                    // 备份完成
                    if (!mBackupManager.IsBackupInProgress()) {
                        // 记录备份完成日志
                        int total = mBackupManager.GetTotalItems();
                        int skipped = mBackupManager.GetSkippedItems();
                        FileLogger::GetInstance().LogBackupEnd(total, skipped);
                        FileLogger::GetInstance().LogSuccess("备份成功完成");
                        FileLogger::GetInstance().EndLog();
                        
                        mBackupState = BACKUP_STATE_DONE;
                    }
                }
            }
            break;
        }
        case BACKUP_STATE_DONE: {
            if (input.data.buttons_d & Input::BUTTON_A) {
                // 返回菜单
                return false;
            }
            break;
        }
        case BACKUP_STATE_ERROR: {
            if (input.data.buttons_d & Input::BUTTON_B) {
                return false;
            }
            break;
        }
    }

    return true;
}
