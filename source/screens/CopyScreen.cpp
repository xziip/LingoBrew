#include "CopyScreen.hpp"
#include "DownloadScreen.hpp"
#include "Gfx.hpp"
#include "common.h"
#include "utils/Utils.hpp"
#include "utils/Language.hpp"
#include "utils/FileLogger.hpp"
#include "utils/Config.hpp"
#include <coreinit/launch.h>   // reboot
#include <sysapp/launch.h>
#include <sys/stat.h>
#include <dirent.h>
#include <whb/log.h>
#include <ctime>
#include <cstring>
#include <functional>

CopyScreen::CopyScreen() {
    // 启动时检测语言文件
    CheckLanguageFiles();
}

CopyScreen::~CopyScreen() {
    // 确保文件被关闭
    EndFileCopy();
}

bool subdirectoryExists = false;

// 检测SD卡上是否有语言文件
bool CopyScreen::CheckLanguageFiles() {
    WHBLogPrintf("开始检测SD卡 title 文件夹...");
    
    mHasLanguageFiles = false;
    mHasOtherFiles = false;
    mDetectedRegion = "";
    
    // 直接检查 SD 卡的 title 文件夹
    const char* titlePath = "fs:/vol/external01/title";
    
    WHBLogPrintf("扫描目录: %s", titlePath);
    
    // 递归扫描函数
    std::function<void(const std::string&, bool&, int&)> scanDir = 
        [&scanDir](const std::string& path, bool& foundAllmessage, int& totalFiles) {
        DIR* dir = opendir(path.c_str());
        if (!dir) {
            WHBLogPrintf("无法打开目录: %s", path.c_str());
            return;
        }
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            // 跳过 . 和 ..
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            
            std::string fullPath = path + "/" + entry->d_name;
            std::string fileName = entry->d_name;
            
            WHBLogPrintf("检查文件: %s", fullPath.c_str());
            
            // 转换为小写进行比较(不区分大小写)
            std::string lowerName = fileName;
            for (char& c : lowerName) {
                c = tolower(c);
            }
            
            // 检查是否是 allmessage.szs (不区分大小写)
            if (lowerName == "allmessage.szs") {
                foundAllmessage = true;
                WHBLogPrintf("找到 allmessage.szs: %s", fullPath.c_str());
            }
            
            totalFiles++;
            
            // 如果是目录,递归扫描
            struct stat st;
            if (stat(fullPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                WHBLogPrintf("进入子目录: %s", fullPath.c_str());
                scanDir(fullPath, foundAllmessage, totalFiles);
            }
        }
        closedir(dir);
    };
    
    // 开始扫描
    bool hasAllmessage = false;
    int fileCount = 0;
    scanDir(titlePath, hasAllmessage, fileCount);
    
    WHBLogPrintf("title 文件夹总文件数: %d, 有 allmessage.szs: %s", 
                fileCount, hasAllmessage ? "是" : "否");
    
    // 设置源目录和目标目录
    mSourceDirectory = titlePath;
    mDestinationDirectory = HAX_DESTINATION_PATH;
    
    if (hasAllmessage) {
        // 有语言文件
        mHasLanguageFiles = true;
        WHBLogPrintf("找到语言文件");
        return true;
    } else if (fileCount > 0) {
        // 有其他文件但没有 allmessage.szs
        mHasOtherFiles = true;
        WHBLogPrintf("找到其他文件但无 allmessage.szs");
        return true;
    }
    
    WHBLogPrintf("title 文件夹为空");
    return false;
}


void CopyScreen::DrawSimpleText(const std::string &text) const {
    Gfx::Print(Gfx::SCREEN_WIDTH / 2, Gfx::SCREEN_HEIGHT / 2, mDefaultFontSize, Gfx::COLOR_TEXT, text, Gfx::ALIGN_CENTER);
}

void CopyScreen::DrawSimpleText(const std::vector<std::string> &items) const {
    int yOff = mDefaultYPos;
    for (const auto &item : items) {
        Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, item, Gfx::ALIGN_VERTICAL);
        yOff += Gfx::GetTextHeight(mDefaultFontSize, item);
    }
}

void CopyScreen::Draw() {
    // 如果有子界面,只绘制子界面
    if (mSubscreen) {
        mSubscreen->Draw();
        return;
    }
    
    Language& lang = Language::GetInstance();
    DrawTopBar(lang.Get("copy.title"));
    switch (mCopyState) {
        case COPY_STATE_CHECKING_FILES: {
            // 检测文件中
            int yOff = Gfx::SCREEN_HEIGHT / 2 - 50;
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, yOff, mDefaultFontSize, Gfx::COLOR_TEXT,
                      lang.Get("copy.checking_files"), Gfx::ALIGN_CENTER);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, yOff, mDefaultFontSize - 10, Gfx::COLOR_ALT_TEXT,
                      lang.Get("common.please_wait"), Gfx::ALIGN_CENTER);
            break;
        }
        
        case COPY_STATE_HAS_LANG_FILES: {
            // 已有语言文件，询问是否下载
            // 显示信息图标
            Gfx::DrawIcon(Gfx::SCREEN_WIDTH / 2, 400, 80, {100, 200, 255, 255}, 0xf05a, Gfx::ALIGN_HORIZONTAL);
            
            // 主要提示
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, 550, mDefaultFontSize, Gfx::COLOR_TEXT,
                      lang.Get("copy.has_lang_files"), Gfx::ALIGN_CENTER);
            
            // 询问
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, 650, mDefaultFontSize - 10, Gfx::COLOR_ALT_TEXT,
                      lang.Get("copy.ask_download_anyway"), Gfx::ALIGN_CENTER);
            
            // 不再提醒提示
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, 700, mDefaultFontSize - 12, {150, 150, 150, 255},
                      lang.Get("copy.press_y_no_remind"), Gfx::ALIGN_CENTER);
            
            // 底边栏: A=下载 B=取消 X=跳过下载直接复制
            DrawBottomBar(lang.Get("copy.press_a_confirm"), 
                         lang.Get("copy.press_x_skip"), 
                         lang.Get("copy.press_b_cancel"));
            break;
        }
        
        case COPY_STATE_HAS_OTHER_FILES: {
            // 有其他文件但无语言文件，询问是否复制
            // 显示警告图标
            Gfx::DrawIcon(Gfx::SCREEN_WIDTH / 2, 400, 80, Gfx::COLOR_WARNING, 0xf071, Gfx::ALIGN_HORIZONTAL);
            
            // 主要提示
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, 550, mDefaultFontSize, Gfx::COLOR_TEXT,
                      lang.Get("copy.has_other_files"), Gfx::ALIGN_CENTER);
            
            // 询问
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, 650, mDefaultFontSize - 10, Gfx::COLOR_ALT_TEXT,
                      lang.Get("copy.ask_copy_other"), Gfx::ALIGN_CENTER);
            
            // 不再提醒提示
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, 700, mDefaultFontSize - 12, {150, 150, 150, 255},
                      lang.Get("copy.press_y_no_remind"), Gfx::ALIGN_CENTER);
            
            // 底边栏: A=复制 B=取消
            DrawBottomBar(lang.Get("copy.press_a_confirm"), 
                         nullptr, 
                         lang.Get("copy.press_b_cancel"));
            break;
        }
        
        case COPY_STATE_NO_FILES: {
            // 没有任何文件，询问是否下载
            // 显示信息图标
            Gfx::DrawIcon(Gfx::SCREEN_WIDTH / 2, 400, 80, {100, 200, 255, 255}, 0xf05a, Gfx::ALIGN_HORIZONTAL);
            
            // 主要提示
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, 550, mDefaultFontSize, Gfx::COLOR_TEXT,
                      lang.Get("copy.no_any_files"), Gfx::ALIGN_CENTER);
            
            // 询问
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, 650, mDefaultFontSize - 10, Gfx::COLOR_ALT_TEXT,
                      lang.Get("copy.ask_download"), Gfx::ALIGN_CENTER);
            
            // 不再提醒提示
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, 700, mDefaultFontSize - 12, {150, 150, 150, 255},
                      lang.Get("copy.press_y_no_remind"), Gfx::ALIGN_CENTER);
            
            // 底边栏: A=下载 B=取消
            DrawBottomBar(lang.Get("copy.press_a_confirm"), 
                         nullptr, 
                         lang.Get("copy.press_b_cancel"));
            break;
        }
        
        case COPY_STATE_SELECT: {
            std::vector<std::string> askText = {
                lang.Get("copy.prepare"),
                " ",
            };
            
            // 绘制普通文本
            int yOff = mDefaultYPos;
            for (const auto &item : askText) {
                Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, item, Gfx::ALIGN_VERTICAL);
                yOff += Gfx::GetTextHeight(mDefaultFontSize, item);
            }
            
            // 显示检测到的区域信息
            if (!mDetectedRegion.empty()) {
                std::string regionInfo = std::string(lang.Get("copy.detected_region")) + ": " + mDetectedRegion;
                Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize - 10, {100, 255, 100, 255}, 
                          regionInfo, Gfx::ALIGN_VERTICAL);
                yOff += Gfx::GetTextHeight(mDefaultFontSize - 10, "");
                yOff += Gfx::GetTextHeight(mDefaultFontSize - 10, "");
            }
            
            // 绘制带发光效果的彩虹色警告（减弱）
            SDL_Color glowColor = Gfx::GetRainbowColor(8.0f);
            std::string warningText = lang.Get("copy.warning");
            
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
            
            // 继续绘制剩余文本
            std::vector<std::string> remainingText = {
                " ",
                lang.Get("copy.ask_backup"),
                lang.Get("copy.backup_path"),
                "",
                lang.Get("copy.select_backup_hint"),
                lang.Get("copy.skip_backup_hint"),
                lang.Get("common.back_hint")
            };
            for (const auto &item : remainingText) {
                Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, item, Gfx::ALIGN_VERTICAL);
                yOff += Gfx::GetTextHeight(mDefaultFontSize, item);
            }
            break;
        }
        case COPY_STATE_ERROR:
            DrawSimpleText(std::string(mError) + ". \ue001" + lang.Get("common.back_hint"));
            break;
        case COPY_STATE_ASK_BACKUP_MODE: {
            std::vector<std::string> askText = {
                lang.Get("copy.select_backup_mode"),
                "",
                std::string("\ue000 ") + lang.Get("copy.backup_full"),
                std::string("  - ") + lang.Get("copy.backup_full_desc1"),
                std::string("  - ") + lang.Get("copy.backup_full_desc2"),
                "",
                std::string("\ue002 ") + lang.Get("copy.backup_selective"),
                std::string("  - ") + lang.Get("copy.backup_selective_desc1"),
                std::string("  - ") + lang.Get("copy.backup_selective_desc2"),
                "",
                lang.Get("common.back_hint")
            };
            DrawSimpleText(askText);
            break;
        }
        case COPY_STATE_DO_BACKUP: {
            std::vector<std::string> status;
            
            if (mBackupManager.IsScanning()) {
                // 正在扫描文件
                int scannedDirs = mBackupManager.GetScannedDirs();
                int foundFiles = mBackupManager.GetTotalItems();
                status.push_back(lang.Get("copy.scanning"));
                status.push_back(Utils::sprintf(lang.Get("copy.scanned_dirs"), scannedDirs));
                status.push_back(Utils::sprintf(lang.Get("copy.found_files"), foundFiles));
                status.push_back(lang.Get("common.please_wait"));
            } else {
                // 正在备份文件
                int total = mBackupManager.GetTotalItems();
                int processed = mBackupManager.GetProcessedItems();
                std::string currentFile = mBackupManager.GetCurrentFile();
                
                if (total > 0) {
                    int percent = (processed * 100) / total;
                    status.push_back(Utils::sprintf(lang.Get("copy.backup_progress"), percent, processed, total));
                }

                if (!currentFile.empty()) {
                    std::string displayPath = currentFile;
                    
                    // 如果路径太长，只显示文件名
                    size_t lastSlash = displayPath.find_last_of("/\\");
                    if (lastSlash != std::string::npos && displayPath.length() > 40) {
                        displayPath = displayPath.substr(lastSlash + 1);
                    }

                    status.push_back(lang.Get("copy.backing_up_file") + displayPath);
                    status.push_back(lang.Get("copy.wait_message"));
                }
            }

            DrawSimpleText(status);
            break;
        }
        case COPY_STATE_CREATE_DIRECTORY:
        case COPY_STATE_OPEN_DIR: {
            std::vector<std::string> status;
            
            if (mIsScanning) {
                status.push_back(lang.Get("copy.scanning"));
            } else {
                // Show progress - 使用已处理的总项目数
                if (mTotalItems > 0) {
                    int percent = (mProcessedItems * 100) / mTotalItems;
                    status.push_back(Utils::sprintf(lang.Get("copy.progress"), percent, mProcessedItems, mTotalItems));
                }

                // Show current operation
                if (!mCurrentFile.empty()) {
                    std::string displayPath = mCurrentFile;
                    
                    // 如果路径太长，只显示文件名
                    size_t lastSlash = displayPath.find_last_of("/\\");
                    if (lastSlash != std::string::npos && displayPath.length() > 40) {
                        displayPath = displayPath.substr(lastSlash + 1);
                    }

                    if (mCurrentIsDirectory) {
                        status.push_back(lang.Get("copy.creating_dir") + displayPath);
                    } else {
                        status.push_back(lang.Get("copy.copying_file") + displayPath);
                        // 如果正在复制文件，显示文件内部进度
                        if (mIsCopyingFile && mCopyFileSize > 0) {
                            int filePercent = (int)((mCopyFileCopied * 100) / mCopyFileSize);
                            status.push_back(Utils::sprintf(lang.Get("copy.file_progress"), filePercent));
                        }
                    }
                    status.push_back(lang.Get("copy.wait_message"));
                }
            }

            DrawSimpleText(status);
            break;
        }
        case COPY_STATE_COPY_FILE:
            DrawSimpleText(lang.Get("copy.copying"));
            break;
        case COPY_STATE_CLEANUP:
            DrawSimpleText(lang.Get("copy.cleanup"));
            break;
        case COPY_STATE_ASK_PLUGIN: {
            std::vector<std::string> askText = {
                lang.Get("copy.title_complete"),
                "",
                lang.Get("copy.fonts_detected"),
                lang.Get("copy.ask_copy_fonts"),
                "",
                lang.Get("copy.copy_fonts_hint"),
                lang.Get("copy.skip_fonts_hint")
            };
            DrawSimpleText(askText);
            break;
        }
        case COPY_STATE_ASK_PLUGIN_BACKUP: {
            std::vector<std::string> askText = {
                lang.Get("copy.prepare_fonts"),
                "",
                lang.Get("copy.ask_backup_fonts"),
                lang.Get("copy.backup_fonts_desc"),
                lang.Get("copy.fonts_backup_path"),
                "",
                lang.Get("copy.backup_fonts_hint"),
                lang.Get("copy.skip_backup_fonts_hint")
            };
            DrawSimpleText(askText);
            break;
        }
        case COPY_STATE_DO_PLUGIN_BACKUP: {
            std::vector<std::string> status;
            
            if (mBackupManager.IsScanning()) {
                // 正在扫描文件
                int scannedDirs = mBackupManager.GetScannedDirs();
                int foundFiles = mBackupManager.GetTotalItems();
                status.push_back(lang.Get("copy.scanning_fonts"));
                status.push_back(Utils::sprintf(lang.Get("copy.scanned_dirs"), scannedDirs));
                status.push_back(Utils::sprintf(lang.Get("copy.found_files"), foundFiles));
                status.push_back(lang.Get("common.please_wait"));
            } else {
                // 正在备份文件
                int total = mBackupManager.GetTotalItems();
                int processed = mBackupManager.GetProcessedItems();
                std::string currentFile = mBackupManager.GetCurrentFile();
                
                if (total > 0) {
                    int percent = (processed * 100) / total;
                    status.push_back(Utils::sprintf(lang.Get("copy.fonts_backup_progress"), percent, processed, total));
                }

                if (!currentFile.empty()) {
                    std::string displayPath = currentFile;
                    
                    // 如果路径太长，只显示文件名
                    size_t lastSlash = displayPath.find_last_of("/\\");
                    if (lastSlash != std::string::npos && displayPath.length() > 40) {
                        displayPath = displayPath.substr(lastSlash + 1);
                    }

                    status.push_back(lang.Get("copy.backing_up_file") + displayPath);
                    status.push_back(lang.Get("copy.wait_message"));
                }
            }

            DrawSimpleText(status);
            break;
        }
        case COPY_STATE_DONE: {
            auto userFriendlyDestinationPath = "mlc:/sys/title/";
            DrawSimpleText((std::vector<std::string>){lang.Get("copy.complete"),
                                                      "",
                                                      lang.Get("copy.files_copied"),
                                                      userFriendlyDestinationPath,
                                                      "",
                                                      lang.Get("copy.press_a_reboot"),
                                                      lang.Get("copy.press_b_return")});
            break;
        }
    }

    // 根据状态显示不同的底部提示
    // 注意: COPY_STATE_HAS_LANG_FILES、COPY_STATE_HAS_OTHER_FILES、COPY_STATE_NO_FILES
    // 这三个状态已经在各自的 case 中绘制了底边栏，不需要再次绘制
    if (mCopyState != COPY_STATE_HAS_LANG_FILES && 
        mCopyState != COPY_STATE_HAS_OTHER_FILES && 
        mCopyState != COPY_STATE_NO_FILES) {
        
        const char* leftHint = nullptr;
        std::string centerHintStr = lang.Get("common.exit");
        const char* centerHint = centerHintStr.c_str();
        
        // 准备右侧提示字符串
        std::string rightHintStr;
        
        if (mCopyState == COPY_STATE_SELECT) {
            rightHintStr = std::string(lang.Get("copy.backup_hint")) + " / " + 
                          lang.Get("copy.skip_hint") + " / " + 
                          lang.Get("common.back_hint");
        } else if (mCopyState == COPY_STATE_ASK_BACKUP_MODE) {
            rightHintStr = std::string(lang.Get("copy.full_hint")) + " / " + 
                          lang.Get("copy.selective_hint") + " / " + 
                          lang.Get("common.back_hint");
        } else if (mCopyState == COPY_STATE_ASK_PLUGIN) {
            rightHintStr = std::string(lang.Get("copy.copy_hint")) + " / " + 
                          lang.Get("copy.skip_hint_B");
        } else if (mCopyState == COPY_STATE_ASK_PLUGIN_BACKUP) {
            rightHintStr = std::string(lang.Get("copy.backup_hint")) + " / " + 
                          lang.Get("copy.skip_hint_B");
        } else if (mCopyState == COPY_STATE_DONE) {
            rightHintStr = std::string(lang.Get("copy.reboot_hint")) + " / " + 
                          lang.Get("copy.menu_hint");
        } else {
            rightHintStr = lang.Get("common.back_hint");
        }
        
        const char* rightHint = rightHintStr.c_str();
        
        DrawBottomBar(leftHint, centerHint, rightHint);
    }
}

void CopyScreen::ScanDirectory(const std::string& srcPath, const std::string& dstPath) {
    DIR* dir = opendir(srcPath.c_str());
    if (!dir) return;

    struct dirent* dp;
    while ((dp = readdir(dir)) != nullptr) {
        std::string name = dp->d_name;
        if (name == "." || name == "..") continue;

        std::string fullSrcPath = srcPath + "/" + name;
        std::string fullDstPath = dstPath + "/" + name;
        
        struct stat filestat;
        if (stat(fullSrcPath.c_str(), &filestat) == 0) {
            bool isDir = (filestat.st_mode & S_IFMT) == S_IFDIR;
            mPendingFiles.push(FileEntry(fullSrcPath, isDir));
            
            // 只统计文件，不统计目录
            if (!isDir) {
                mTotalItems++;
            }
            
            if (isDir) {
                // 递归扫描子目录
                ScanDirectory(fullSrcPath, fullDstPath);
            }
        }
    }
    closedir(dir);
}

bool CopyScreen::ProcessNextFile() {
    if (mPendingFiles.empty()) {
        return true; // 完成
    }

    FileEntry entry = mPendingFiles.front();
    mPendingFiles.pop();
    
    // 先更新当前状态，这样UI能立即显示
    mCurrentIsDirectory = entry.isDirectory;
    
    // 使用相对于源目录的路径来显示
    std::string relativePath = entry.path;
    if (relativePath.length() > mSourceDirectory.length()) {
        relativePath = relativePath.substr(mSourceDirectory.length());
        // 移除开头的斜杠
        if (!relativePath.empty() && (relativePath[0] == '/' || relativePath[0] == '\\')) {
            relativePath = relativePath.substr(1);
        }
    }
    mCurrentFile = relativePath;

    std::string dstPath = mDestinationDirectory + entry.path.substr(mSourceDirectory.length());

    // 执行实际的复制操作
    if (entry.isDirectory) {
        if (!Utils::CreateSubfolder(dstPath)) {
            mError = "创建目录失败: " + dstPath;
            return false;
        }
        // 目录创建完成，不增加计数（只统计文件）
    } else {
        // 文件复制：开始分块复制，不在这里增加计数
        // 计数将在文件复制完成后增加
        if (!StartFileCopy(entry.path, dstPath)) {
            return false;
        }
        // 返回 true 但不增加计数，等待分块复制完成
        return true;
    }

    return true;
}

bool CopyScreen::StartFileCopy(const std::string& srcPath, const std::string& dstPath) {
    // 打开源文件
    mCopySourceFile = fopen(srcPath.c_str(), "rb");
    if (!mCopySourceFile) {
        mError = "无法打开源文件: " + srcPath;
        return false;
    }
    
    // 获取文件大小
    fseek(mCopySourceFile, 0, SEEK_END);
    mCopyFileSize = ftell(mCopySourceFile);
    fseek(mCopySourceFile, 0, SEEK_SET);
    
    // 打开目标文件
    mCopyDestFile = fopen(dstPath.c_str(), "wb");
    if (!mCopyDestFile) {
        fclose(mCopySourceFile);
        mCopySourceFile = nullptr;
        mError = "无法创建目标文件: " + dstPath;
        return false;
    }
    
    mCopyFileCopied = 0;
    mIsCopyingFile = true;
    return true;
}

bool CopyScreen::ContinueFileCopy() {
    if (!mIsCopyingFile || !mCopySourceFile || !mCopyDestFile) {
        return false;
    }
    
    // 每次复制 256KB
    const size_t CHUNK_SIZE = 256 * 1024;
    char buffer[CHUNK_SIZE];
    
    size_t bytesRead = fread(buffer, 1, CHUNK_SIZE, mCopySourceFile);
    if (bytesRead > 0) {
        size_t bytesWritten = fwrite(buffer, 1, bytesRead, mCopyDestFile);
        if (bytesWritten != bytesRead) {
            mError = "写入文件失败";
            EndFileCopy();
            return false;
        }
        mCopyFileCopied += bytesRead;
    }
    
    // 检查是否复制完成
    if (mCopyFileCopied >= mCopyFileSize || feof(mCopySourceFile)) {
        EndFileCopy();
        return true; // 复制完成
    }
    
    return true; // 继续复制
}

void CopyScreen::EndFileCopy() {
    if (mCopySourceFile) {
        fclose(mCopySourceFile);
        mCopySourceFile = nullptr;
    }
    if (mCopyDestFile) {
        fclose(mCopyDestFile);
        mCopyDestFile = nullptr;
    }
    mIsCopyingFile = false;
    mCopyFileSize = 0;
    mCopyFileCopied = 0;
}

bool CopyScreen::Update(Input &input) {
    // 如果有子界面,将输入传递给子界面
    if (mSubscreen) {
        if (!mSubscreen->Update(input)) {
            // 子界面请求关闭
            mSubscreen.reset();
        }
        return true;  // 保持CopyScreen激活
    }
    
    switch (mCopyState) {
        case COPY_STATE_CHECKING_FILES: {
            // 检测完成后切换状态
            if (!Config::GetInstance().ShowCopyFileDetection()) {
                // 如果用户选择不再显示提醒,直接进入选择界面
                mCopyState = COPY_STATE_SELECT;
            } else if (mHasLanguageFiles) {
                mCopyState = COPY_STATE_HAS_LANG_FILES;
            } else if (mHasOtherFiles) {
                mCopyState = COPY_STATE_HAS_OTHER_FILES;
            } else {
                mCopyState = COPY_STATE_NO_FILES;
            }
            break;
        }
        
        case COPY_STATE_HAS_LANG_FILES: {
            // 已有语言文件，询问是否下载
            if (input.data.buttons_d & Input::BUTTON_A) {
                // 确认下载 - 跳转到下载页面
                mSubscreen = std::make_unique<DownloadScreen>();
                return true;
            } else if (input.data.buttons_d & Input::BUTTON_B) {
                // 取消 - 返回菜单
                return false;
            } else if (input.data.buttons_d & Input::BUTTON_X) {
                // 跳过 - 进入复制流程
                mCopyState = COPY_STATE_SELECT;
            } else if (input.data.buttons_d & Input::BUTTON_Y) {
                // 不再显示提醒 - 保存配置并进入选择界面
                Config::GetInstance().SetShowCopyFileDetection(false);
                mCopyState = COPY_STATE_SELECT;
            }
            break;
        }
        
        case COPY_STATE_HAS_OTHER_FILES: {
            // 有其他文件但无语言文件，询问是否复制
            if (input.data.buttons_d & Input::BUTTON_A) {
                // 确认复制 - 进入复制流程
                mCopyState = COPY_STATE_SELECT;
            } else if (input.data.buttons_d & Input::BUTTON_B) {
                // 取消 - 返回菜单
                return false;
            } else if (input.data.buttons_d & Input::BUTTON_Y) {
                // 不再显示提醒 - 保存配置并进入选择界面
                Config::GetInstance().SetShowCopyFileDetection(false);
                mCopyState = COPY_STATE_SELECT;
            }
            break;
        }
        
        case COPY_STATE_NO_FILES: {
            // 没有任何文件，询问是否下载
            if (input.data.buttons_d & Input::BUTTON_A) {
                // 确认下载 - 跳转到下载页面
                mSubscreen = std::make_unique<DownloadScreen>();
                return true;
            } else if (input.data.buttons_d & Input::BUTTON_B) {
                // 取消 - 返回菜单
                return false;
            } else if (input.data.buttons_d & Input::BUTTON_Y) {
                // 不再显示提醒 - 保存配置并返回菜单
                Config::GetInstance().SetShowCopyFileDetection(false);
                return false;
            }
            break;
        }
        
        case COPY_STATE_SELECT: {
            if (input.data.buttons_d & Input::BUTTON_A) {
                // 用户选择备份，进入备份模式选择
                mCopyState = COPY_STATE_ASK_BACKUP_MODE;
            } else if (input.data.buttons_d & Input::BUTTON_X) {
                // 用户选择跳过备份，直接开始复制
                mBackupMode = BACKUP_MODE_NONE;
                mIsScanning = true;
                
                // 启动复制日志
                FileLogger::GetInstance().StartLog("copy");
                FileLogger::GetInstance().Log("========== 开始复制操作 ==========");
                FileLogger::GetInstance().Log("操作类型: 跳过备份直接复制");
                FileLogger::GetInstance().Log("检测到的区域: %s", mDetectedRegion.c_str());
                FileLogger::GetInstance().Log("源目录: %s", mSourceDirectory.c_str());
                FileLogger::GetInstance().Log("目标目录: %s", mDestinationDirectory.c_str());
                
                mCopyState = COPY_STATE_CREATE_DIRECTORY;
                
                // 检查字体目录是否存在
                DIR* pluginDir = opendir(HAX_PLUGINS_SOURCE_PATH);
                if (pluginDir) {
                    subdirectoryExists = true;
                    closedir(pluginDir);
                }
                
                // 创建根目录
                if (!Utils::CreateSubfolder(mDestinationDirectory)) {
                    mError = "创建目标目录失败";
                    mCopyState = COPY_STATE_ERROR;
                    break;
                }
                // 开始扫描文件
                ScanDirectory(mSourceDirectory, mDestinationDirectory);
                mIsScanning = false;
                
                // 预览第一个文件（如果有）
                if (!mPendingFiles.empty()) {
                    FileEntry& firstEntry = mPendingFiles.front();
                    std::string relativePath = firstEntry.path;
                    if (relativePath.length() > mSourceDirectory.length()) {
                        relativePath = relativePath.substr(mSourceDirectory.length());
                        if (!relativePath.empty() && (relativePath[0] == '/' || relativePath[0] == '\\')) {
                            relativePath = relativePath.substr(1);
                        }
                    }
                    mCurrentFile = relativePath;
                    mCurrentIsDirectory = firstEntry.isDirectory;
                }
            } else if (input.data.buttons_d & Input::BUTTON_B) {
                // 用户选择返回菜单
                return false;
            }
            break;
        }
        case COPY_STATE_ASK_BACKUP_MODE: {
            // 等待用户选择备份模式
            if (input.data.buttons_d & Input::BUTTON_A) {
                // 备份整个MLC
                mBackupMode = BACKUP_MODE_ALL;
                mCopyState = COPY_STATE_DO_BACKUP;
                
                // 使用BackupManager开始备份
                if (!mBackupManager.StartBackup(HAX_DESTINATION_PATH, BACKUP_TITLE_PATH)) {
                    mError = "创建备份目录失败";
                    mCopyState = COPY_STATE_ERROR;
                    break;
                }
            } else if (input.data.buttons_d & Input::BUTTON_X) {
                // 只备份将被覆盖的文件
                mBackupMode = BACKUP_MODE_OVERWRITE;
                mCopyState = COPY_STATE_DO_BACKUP;
                
                // 使用选择性备份：比较MLC和SD卡，只备份将被覆盖的文件
                if (!mBackupManager.StartSelectiveBackup(HAX_DESTINATION_PATH, HAX_SOURCE_PATH, BACKUP_TITLE_PATH)) {
                    mError = "创建备份目录失败";
                    mCopyState = COPY_STATE_ERROR;
                    break;
                }
            } else if (input.data.buttons_d & Input::BUTTON_B) {
                // 返回上一级
                mCopyState = COPY_STATE_SELECT;
            }
            break;
        }
        case COPY_STATE_DO_BACKUP: {
            // 使用BackupManager更新备份进度
            if (mBackupManager.IsBackupInProgress()) {
                if (!mBackupManager.UpdateBackup()) {
                    // 备份完成或出错
                    if (!mBackupManager.IsBackupInProgress()) {
                        // 备份完成，开始正常复制流程
                        mIsScanning = true;
                        
                        // 启动复制日志
                        FileLogger::GetInstance().StartLog("copy");
                        FileLogger::GetInstance().Log("========== 开始复制操作 ==========");
                        FileLogger::GetInstance().Log("操作类型: 备份后复制");
                        FileLogger::GetInstance().Log("检测到的区域: %s", mDetectedRegion.c_str());
                        FileLogger::GetInstance().Log("源目录: %s", mSourceDirectory.c_str());
                        FileLogger::GetInstance().Log("目标目录: %s", mDestinationDirectory.c_str());
                        
                        mCopyState = COPY_STATE_CREATE_DIRECTORY;
                        
                        // 检查字体目录是否存在
                        DIR* pluginDir = opendir(HAX_PLUGINS_SOURCE_PATH);
                        if (pluginDir) {
                            subdirectoryExists = true;
                            closedir(pluginDir);
                        }
                        
                        // 创建根目录
                        if (!Utils::CreateSubfolder(mDestinationDirectory)) {
                            mError = "创建目标目录失败";
                            mCopyState = COPY_STATE_ERROR;
                            break;
                        }
                        // 开始扫描文件
                        ScanDirectory(mSourceDirectory, mDestinationDirectory);
                        mIsScanning = false;
                        
                        // 预览第一个文件（如果有）
                        if (!mPendingFiles.empty()) {
                            FileEntry& firstEntry = mPendingFiles.front();
                            std::string relativePath = firstEntry.path;
                            if (relativePath.length() > mSourceDirectory.length()) {
                                relativePath = relativePath.substr(mSourceDirectory.length());
                                if (!relativePath.empty() && (relativePath[0] == '/' || relativePath[0] == '\\')) {
                                    relativePath = relativePath.substr(1);
                                }
                            }
                            mCurrentFile = relativePath;
                            mCurrentIsDirectory = firstEntry.isDirectory;
                        }
                    }
                }
            }
            break;
        }
        case COPY_STATE_CREATE_DIRECTORY: {
            // 如果正在复制文件，继续复制
            if (mIsCopyingFile) {
                if (!ContinueFileCopy()) {
                    mCopyState = COPY_STATE_ERROR;
                    break;
                }
                // 如果文件复制完成
                if (!mIsCopyingFile) {
                    mProcessedItems++; // 文件复制完成，增加计数
                }
                break;
            }
            
            // 没有在复制文件，处理下一个项目
            // 动态处理：目录快速批量处理，文件启动复制
            int processed = 0;
            int maxBatchSize = 20; // 最多批量处理20个目录
            
            while (!mPendingFiles.empty() && processed < maxBatchSize) {
                // 查看下一个项目
                bool isNextDir = mPendingFiles.front().isDirectory;
                
                if (!ProcessNextFile()) {
                    mCopyState = COPY_STATE_ERROR;
                    break;
                }
                
                processed++;
                
                // 如果启动了文件复制，退出循环等待下一帧继续
                if (mIsCopyingFile) {
                    break;
                }
                
                // 如果处理的不是目录，退出循环
                if (!isNextDir) {
                    break;
                }
            }
            
            // 检查是否完成
            if (mPendingFiles.empty() && !mIsCopyingFile) {
                mCopyState = COPY_STATE_CLEANUP;
            }
            break;
        }
        case COPY_STATE_OPEN_DIR:
            mCopyState = COPY_STATE_COPY_FILE;
            break;
        case COPY_STATE_COPY_FILE:
            mCopyState = COPY_STATE_CLEANUP;
            break;
        case COPY_STATE_CLEANUP:
            // 检查是否有字体目录需要复制
            if (subdirectoryExists) {
                mCopyState = COPY_STATE_ASK_PLUGIN;
            } else {
                FileLogger::GetInstance().LogSuccess("复制操作完成! 共复制 %d 个项目", mTotalItems);
                FileLogger::GetInstance().EndLog();
                mCopyState = COPY_STATE_DONE;
            }
            break;
        case COPY_STATE_ASK_PLUGIN:
            // 等待用户选择
            if (input.data.buttons_d & Input::BUTTON_A) {
                // 用户选择复制字体，先询问是否备份
                mCopyState = COPY_STATE_ASK_PLUGIN_BACKUP;
            } else if (input.data.buttons_d & Input::BUTTON_B) {
                // 用户选择跳过
                subdirectoryExists = false;
                FileLogger::GetInstance().LogSuccess("复制操作完成! 共复制 %d 个项目 (跳过字体)", mTotalItems);
                FileLogger::GetInstance().EndLog();
                mCopyState = COPY_STATE_DONE;
            }
            break;
        case COPY_STATE_ASK_PLUGIN_BACKUP:
            // 等待用户选择是否备份字体
            if (input.data.buttons_d & Input::BUTTON_A) {
                // 用户选择备份字体（使用选择性备份）
                mCopyState = COPY_STATE_DO_PLUGIN_BACKUP;
                
                // 使用BackupManager开始选择性备份字体
                if (!mBackupManager.StartSelectiveBackup(HAX_PLUGINS_DESTINATION_PATH, HAX_PLUGINS_SOURCE_PATH, BACKUP_FONTS_PATH)) {
                    mError = "创建字体备份目录失败";
                    mCopyState = COPY_STATE_ERROR;
                    break;
                }
            } else if (input.data.buttons_d & Input::BUTTON_B) {
                // 用户选择跳过备份，直接复制字体
                mSourceDirectory      = HAX_PLUGINS_SOURCE_PATH;
                mDestinationDirectory = HAX_PLUGINS_DESTINATION_PATH;
                subdirectoryExists    = false;
                // 重置计数器和状态
                mTotalItems = 0;
                mProcessedItems = 0;
                mCurrentFile = "";
                mCurrentIsDirectory = false;
                // 清空待处理文件队列
                while (!mPendingFiles.empty()) {
                    mPendingFiles.pop();
                }
                mIsScanning = true;
                mCopyState = COPY_STATE_CREATE_DIRECTORY;
                // 开始扫描新目录
                if (!Utils::CreateSubfolder(mDestinationDirectory)) {
                    mError = "创建字体目标目录失败";
                    mCopyState = COPY_STATE_ERROR;
                    break;
                }
                ScanDirectory(mSourceDirectory, mDestinationDirectory);
                mIsScanning = false;
                
                // 预览第一个文件（如果有）
                if (!mPendingFiles.empty()) {
                    FileEntry& firstEntry = mPendingFiles.front();
                    std::string relativePath = firstEntry.path;
                    if (relativePath.length() > mSourceDirectory.length()) {
                        relativePath = relativePath.substr(mSourceDirectory.length());
                        if (!relativePath.empty() && (relativePath[0] == '/' || relativePath[0] == '\\')) {
                            relativePath = relativePath.substr(1);
                        }
                    }
                    mCurrentFile = relativePath;
                    mCurrentIsDirectory = firstEntry.isDirectory;
                }
            }
            break;
        case COPY_STATE_DO_PLUGIN_BACKUP: {
            // 使用BackupManager更新字体备份进度
            if (mBackupManager.IsBackupInProgress()) {
                if (!mBackupManager.UpdateBackup()) {
                    // 备份完成，开始复制字体
                    mSourceDirectory      = HAX_PLUGINS_SOURCE_PATH;
                    mDestinationDirectory = HAX_PLUGINS_DESTINATION_PATH;
                    subdirectoryExists    = false;
                    // 重置计数器和状态
                    mTotalItems = 0;
                    mProcessedItems = 0;
                    mCurrentFile = "";
                    mCurrentIsDirectory = false;
                    // 清空待处理文件队列
                    while (!mPendingFiles.empty()) {
                        mPendingFiles.pop();
                    }
                    mIsScanning = true;
                    mCopyState = COPY_STATE_CREATE_DIRECTORY;
                    // 开始扫描新目录
                    if (!Utils::CreateSubfolder(mDestinationDirectory)) {
                        mError = "创建字体目标目录失败";
                        mCopyState = COPY_STATE_ERROR;
                        break;
                    }
                    ScanDirectory(mSourceDirectory, mDestinationDirectory);
                    mIsScanning = false;
                    
                    // 预览第一个文件（如果有）
                    if (!mPendingFiles.empty()) {
                        FileEntry& firstEntry = mPendingFiles.front();
                        std::string relativePath = firstEntry.path;
                        if (relativePath.length() > mSourceDirectory.length()) {
                            relativePath = relativePath.substr(mSourceDirectory.length());
                            if (!relativePath.empty() && (relativePath[0] == '/' || relativePath[0] == '\\')) {
                                relativePath = relativePath.substr(1);
                            }
                        }
                        mCurrentFile = relativePath;
                        mCurrentIsDirectory = firstEntry.isDirectory;
                    }
                }
            }
            break;
        }
        case COPY_STATE_DONE:
            if (input.data.buttons_d & Input::BUTTON_A) {
                // 重启系统
                OSLaunchTitlel(OS_TITLE_ID_REBOOT, 0);
                return true; // 保持在当前界面，等待重启
            } else if (input.data.buttons_d & Input::BUTTON_B) {
                // 返回菜单
                return false;
            }
            break;
        case COPY_STATE_ERROR:
            if (!mErrorLogged) {
                FileLogger::GetInstance().LogError("复制失败: %s", mError.c_str());
                FileLogger::GetInstance().EndLog();
                mErrorLogged = true;
            }
            if (input.data.buttons_d & Input::BUTTON_B) {
                return false;
            }
            break;
    }

    return true;
}
