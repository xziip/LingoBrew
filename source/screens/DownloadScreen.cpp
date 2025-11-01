#include "DownloadScreen.hpp"
#include "Gfx.hpp"
#include "utils/Language.hpp"
#include "utils/FileLogger.hpp"
#include "common.h"
#include <sstream>
#include <iomanip>
#include <whb/log.h>
#include <sys/stat.h>
#include <curl/curl.h>

// 语言文件版本
#define LANG_FILE_VERSION "1.04"

// 下载目录（与DownloadManager.cpp保持一致）
#define TEMP_DOWNLOAD_DIR "fs:/vol/external01/wiiu/apps/LingoBrew/"

// 镜像源基础URL
#define MIRROR_ORIGINAL_BASE "https://github.com/xziip/wiiu-lang-files/releases/download/lang/"
#define MIRROR_ACCELERATED_BASE "https://gitee.com/xziip/wiiu-lang-files/releases/download/lang/"

DownloadScreen::DownloadScreen()
    : mScreenState(STATE_SELECT_REGION)
    , mSelectedRegion(REGION_US)
    , mSelectedMirror(MIRROR_ORIGINAL)
    , mSelectedLanguage(LANG_CHINESE)
    , mProgress(0.0f)
    , mDownloadedSize(0)
    , mTotalSize(0)
    , mSpeed(0)
    , mShowCancelConfirm(false)
    , mSHA256Verified(false)
{
    // 根据当前语言自动选择镜像类型
    Language& lang = Language::GetInstance();
    if (lang.GetCurrentLanguage() == LanguageCode::CHINESE) {
        mSelectedMirror = MIRROR_ACCELERATED;  // 中文用户默认加速镜像
    } else {
        mSelectedMirror = MIRROR_ORIGINAL;     // 其他用户默认原镜像
    }
    
    // 设置下载管理器回调
    mDownloadManager.SetProgressCallback([this](float progress, long downloaded, long total) {
        mProgress = progress;
        mDownloadedSize = downloaded;
        mTotalSize = total;
        mSpeed = mDownloadManager.GetSpeed();
    });
    
    mDownloadManager.SetStateCallback([this](DownloadManager::DownloadState state, const std::string& message) {
        mStatusMessage = message;
        
        switch (state) {
            case DownloadManager::DOWNLOAD_EXTRACTING:
                mScreenState = STATE_EXTRACTING;
                FileLogger::GetInstance().Log("状态切换: 开始解压");
                break;
            case DownloadManager::DOWNLOAD_COMPLETE:
                mScreenState = STATE_COMPLETE;
                FileLogger::GetInstance().LogSuccess("下载和解压全部完成!");
                FileLogger::GetInstance().EndLog();
                break;
            case DownloadManager::DOWNLOAD_ERROR:
                mScreenState = STATE_ERROR;
                FileLogger::GetInstance().LogError("下载失败: %s", message.c_str());
                FileLogger::GetInstance().EndLog();
                break;
            default:
                if (mScreenState != STATE_EXTRACTING) {
                    mScreenState = STATE_DOWNLOADING;
                }
                break;
        }
    });
}

DownloadScreen::~DownloadScreen() {
    mDownloadManager.CancelDownload();
}

std::string DownloadScreen::GenerateDownloadUrl() {
    // 生成文件名: 区域-语言-版本.zip
    // 例如: US-CH-1.04.zip
    std::string regionCode;
    switch (mSelectedRegion) {
        case REGION_EU: regionCode = "EU"; break;
        case REGION_US: regionCode = "US"; break;
        case REGION_JP: regionCode = "JP"; break;
        default: regionCode = "JP"; break;
    }
    
    std::string langCode;
    switch (mSelectedLanguage) {
        case LANG_CHINESE: langCode = "CH"; break;
        default: langCode = "CH"; break;
    }
    
    std::string filename = regionCode + "-" + langCode + "-" + LANG_FILE_VERSION + ".zip";
    
    // 选择镜像基础URL
    std::string baseUrl;
    if (mSelectedMirror == MIRROR_ACCELERATED) {
        baseUrl = MIRROR_ACCELERATED_BASE;
        WHBLogPrintf("使用加速镜像源: %s", baseUrl.c_str());
        FileLogger::GetInstance().Log("使用加速镜像源: %s", baseUrl.c_str());
    } else {
        baseUrl = MIRROR_ORIGINAL_BASE;
        WHBLogPrintf("使用原始镜像源: %s", baseUrl.c_str());
        FileLogger::GetInstance().Log("使用原始镜像源: %s", baseUrl.c_str());
    }
    
    std::string fullUrl = baseUrl + filename;
    WHBLogPrintf("完整下载URL: %s", fullUrl.c_str());
    FileLogger::GetInstance().Log("完整下载URL: %s", fullUrl.c_str());
    
    return fullUrl;
}

void DownloadScreen::StartDownload() {
    // 启动下载日志
    FileLogger::GetInstance().StartLog("download");
    
    mScreenState = STATE_DOWNLOADING;
    mProgress = 0.0f;
    mDownloadedSize = 0;
    mTotalSize = 0;
    
    std::string url = GenerateDownloadUrl();
    
    FileLogger::GetInstance().Log("开始下载任务");
    FileLogger::GetInstance().Log("区域: %s", GetRegionName(mSelectedRegion).c_str());
    FileLogger::GetInstance().Log("语言: %s", GetLanguageName(mSelectedLanguage).c_str());
    FileLogger::GetInstance().Log("镜像: %s", GetMirrorTypeName(mSelectedMirror).c_str());
    
    // 先尝试下载并解析 SHA256.txt
    std::string sha256Value = DownloadAndParseSHA256();
    
    std::vector<std::string> urls = {url};
    
    // 如果获取到 SHA-256 值，则使用校验下载
    if (!sha256Value.empty()) {
        mSHA256Verified = true;
        FileLogger::GetInstance().Log("使用 SHA-256 校验: %s", sha256Value.c_str());
        if (!mDownloadManager.StartDownload(urls, "", sha256Value)) {
            mScreenState = STATE_ERROR;
            mStatusMessage = mDownloadManager.GetError();
            FileLogger::GetInstance().LogError("启动下载失败: %s", mStatusMessage.c_str());
            FileLogger::GetInstance().EndLog();
        }
    } else {
        // 无法获取 SHA-256，不进行校验
        mSHA256Verified = false;
        FileLogger::GetInstance().Log("未获取到 SHA-256 值，跳过校验");
        if (!mDownloadManager.StartDownload(urls, "")) {
            mScreenState = STATE_ERROR;
            mStatusMessage = mDownloadManager.GetError();
            FileLogger::GetInstance().LogError("启动下载失败: %s", mStatusMessage.c_str());
            FileLogger::GetInstance().EndLog();
        }
    }
}

void DownloadScreen::Draw() {
    Language& lang = Language::GetInstance();
    DrawTopBar(lang.Get("download.title"));
    
    int yOff = mDefaultYPos;
    
    switch (mScreenState) {
        case STATE_SELECT_REGION: {
            // 选择系统区域
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT,
                      lang.Get("download.select_region"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // 显示三个选项
            for (int i = 0; i < REGION_COUNT; i++) {
                SDL_Color color = (i == mSelectedRegion) ? Gfx::COLOR_HIGHLIGHTED : Gfx::COLOR_TEXT;
                std::string prefix = (i == mSelectedRegion) ? "> " : "  ";
                std::string regionName = GetRegionName(static_cast<SystemRegion>(i));
                
                Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, color,
                          prefix + regionName, Gfx::ALIGN_VERTICAL);
                yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            }
            
            std::string selectHint = std::string(lang.Get("common.select"));
            std::string confirmHint = std::string(lang.Get("common.confirm"));
            std::string backHint = std::string(lang.Get("common.back"));
            DrawBottomBar(selectHint.c_str(), confirmHint.c_str(), backHint.c_str());
            break;
        }
        
        case STATE_SELECT_MIRROR: {
            // 选择镜像源
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT,
                      lang.Get("download.select_mirror"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // 显示两个选项
            for (int i = 0; i < MIRROR_COUNT; i++) {
                SDL_Color color = (i == mSelectedMirror) ? Gfx::COLOR_HIGHLIGHTED : Gfx::COLOR_TEXT;
                std::string prefix = (i == mSelectedMirror) ? "> " : "  ";
                std::string mirrorName = GetMirrorTypeName(static_cast<MirrorType>(i));
                
                Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, color,
                          prefix + mirrorName, Gfx::ALIGN_VERTICAL);
                yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            }
            
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // 显示推荐提示
            std::string recommendText;
            if (lang.GetCurrentLanguage() == LanguageCode::CHINESE) {
                recommendText = lang.Get("download.recommend_accelerated");
            } else {
                recommendText = lang.Get("download.recommend_original");
            }
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize - 10, {200, 200, 200, 255},
                      recommendText, Gfx::ALIGN_VERTICAL);

            std::string selectHint = std::string(lang.Get("common.select"));
            std::string confirmHint = std::string(lang.Get("common.confirm"));
            std::string backHint = std::string(lang.Get("common.return"));
            DrawBottomBar(selectHint.c_str(), confirmHint.c_str(), backHint.c_str());
            break;
        }
        
        case STATE_SELECT_LANGUAGE: {
            // 选择目标语言
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT,
                      lang.Get("download.select_language"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // 显示语言选项
            for (int i = 0; i < LANG_COUNT; i++) {
                SDL_Color color = (i == mSelectedLanguage) ? Gfx::COLOR_HIGHLIGHTED : Gfx::COLOR_TEXT;
                std::string prefix = (i == mSelectedLanguage) ? "> " : "  ";
                std::string langName = GetLanguageName(static_cast<TargetLanguage>(i));
                
                Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, color,
                          prefix + langName, Gfx::ALIGN_VERTICAL);
                yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            }
            
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // 显示即将下载的文件信息
            std::string filename = GetRegionName(mSelectedRegion) + "-" + 
                                  GetLanguageName(mSelectedLanguage) + "-" + LANG_FILE_VERSION;
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize - 10, {200, 200, 200, 255},
                      std::string(lang.Get("download.file_to_download")) + ": " + filename + ".zip", Gfx::ALIGN_VERTICAL);
            
            std::string startHint = std::string(lang.Get("common.button_a")) + " " + lang.Get("download.start");
            std::string backHint = std::string(lang.Get("common.back"));
            DrawBottomBar(nullptr, startHint.c_str(), backHint.c_str());
            break;
        }
        
        case STATE_DOWNLOADING: {
            // 下载中
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT,
                      lang.Get("download.downloading"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // 进度条
            int progressBarWidth = Gfx::SCREEN_WIDTH - mDefaultXPos * 2;
            int progressBarHeight = 40;
            int progressBarY = yOff;
            
            // 背景
            Gfx::DrawRectFilled(mDefaultXPos, progressBarY, progressBarWidth, progressBarHeight,
                               {50, 50, 50, 255});
            
            // 进度
            int filledWidth = static_cast<int>(progressBarWidth * mProgress);
            Gfx::DrawRectFilled(mDefaultXPos, progressBarY, filledWidth, progressBarHeight,
                               {0, 150, 255, 255});
            
            // 百分比
            std::stringstream ss;
            ss << std::fixed << std::setprecision(1) << (mProgress * 100.0f) << "%";
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, progressBarY + progressBarHeight / 2,
                      mDefaultFontSize, {255, 255, 255, 255}, ss.str(), Gfx::ALIGN_CENTER);
            
            yOff += progressBarHeight;
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // 下载信息
            std::string sizeText = FormatFileSize(mDownloadedSize) + " / " + FormatFileSize(mTotalSize);
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT,
                      sizeText, Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            std::string speedText = std::string(lang.Get("download.speed")) + ": " + FormatSpeed(mSpeed);
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT,
                      speedText, Gfx::ALIGN_VERTICAL);
            
            std::string cancelHint = std::string(lang.Get("common.button_b")) + " " + lang.Get("download.cancel");
            DrawBottomBar(nullptr, cancelHint.c_str(), nullptr);
            break;
        }
        
        case STATE_EXTRACTING: {
            // 解压中
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT,
                      lang.Get("download.extracting"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // 进度条
            int progressBarWidth = Gfx::SCREEN_WIDTH - mDefaultXPos * 2;
            int progressBarHeight = 40;
            int progressBarY = yOff;
            
            // 背景
            Gfx::DrawRectFilled(mDefaultXPos, progressBarY, progressBarWidth, progressBarHeight,
                               {50, 50, 50, 255});
            
            // 进度
            int filledWidth = static_cast<int>(progressBarWidth * mProgress);
            Gfx::DrawRectFilled(mDefaultXPos, progressBarY, filledWidth, progressBarHeight,
                               {0, 255, 150, 255});  // 绿色表示解压
            
            // 百分比
            std::stringstream ss;
            ss << std::fixed << std::setprecision(1) << (mProgress * 100.0f) << "%";
            Gfx::Print(Gfx::SCREEN_WIDTH / 2, progressBarY + progressBarHeight / 2,
                      mDefaultFontSize, {255, 255, 255, 255}, ss.str(), Gfx::ALIGN_CENTER);
            
            yOff += progressBarHeight;
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // 解压信息
            if (mTotalSize > 0) {
                std::string filesText = std::to_string(mDownloadedSize) + " / " + std::to_string(mTotalSize) + " " + lang.Get("download.files");
                Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT,
                          filesText, Gfx::ALIGN_VERTICAL);
            } else {
                Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT,
                          lang.Get("common.please_wait"), Gfx::ALIGN_VERTICAL);
            }
            
            DrawBottomBar(nullptr, lang.Get("common.please_wait"), nullptr);
            break;
        }
        
        case STATE_COMPLETE: {
            // 完成
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, {0, 255, 0, 255},
                      lang.Get("download.complete"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            // 显示成功信息
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize - 10, {200, 200, 200, 255},
                      "语言文件已成功下载并解压到 SD 卡", Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize - 10, "");
            yOff += Gfx::GetTextHeight(mDefaultFontSize - 10, "");
            
            // 使用 Font Awesome 图标和文字
            int iconSize = mDefaultFontSize - 10;
            int iconX = mDefaultXPos;
            int textX = iconX + iconSize + 10;  // 图标右侧留10像素间距
            
            // 下载完成
            Gfx::DrawIcon(iconX, yOff, iconSize, {100, 255, 100, 255}, 0xf00c, Gfx::ALIGN_VERTICAL);
            Gfx::Print(textX, yOff, iconSize, {100, 255, 100, 255},
                      "下载完成", Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(iconSize, "");
            
            // 解压完成
            Gfx::DrawIcon(iconX, yOff, iconSize, {100, 255, 100, 255}, 0xf00c, Gfx::ALIGN_VERTICAL);
            Gfx::Print(textX, yOff, iconSize, {100, 255, 100, 255},
                      "解压完成", Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(iconSize, "");
            
            // SHA-256 校验状态
            if (mSHA256Verified) {
                Gfx::DrawIcon(iconX, yOff, iconSize, {100, 255, 100, 255}, 0xf00c, Gfx::ALIGN_VERTICAL);
                Gfx::Print(textX, yOff, iconSize, {100, 255, 100, 255},
                          "SHA-256校验通过", Gfx::ALIGN_VERTICAL);
            } else {
                Gfx::DrawIcon(iconX, yOff, iconSize, {255, 200, 0, 255}, 0xf071, Gfx::ALIGN_VERTICAL);
                Gfx::Print(textX, yOff, iconSize, {255, 200, 0, 255},
                          "未进行SHA-256校验", Gfx::ALIGN_VERTICAL);
            }
            yOff += Gfx::GetTextHeight(iconSize, "");
            yOff += Gfx::GetTextHeight(iconSize, "");
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize - 10, {255, 255, 100, 255},
                      "提示：现在可以使用复制界面替换系统语言文件了", Gfx::ALIGN_VERTICAL);

            std::string returnHint = std::string(lang.Get("common.return_menu"));
            DrawBottomBar(nullptr, nullptr, returnHint.c_str());
            break;
        }
        
        case STATE_ERROR: {
            // 错误（黄色警告）
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, {255, 200, 0, 255},
                      lang.Get("download.error"), Gfx::ALIGN_VERTICAL);
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            yOff += Gfx::GetTextHeight(mDefaultFontSize, "");
            
            Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT,
                      mStatusMessage, Gfx::ALIGN_VERTICAL);
            
            std::string returnHint = std::string(lang.Get("common.return"));
            DrawBottomBar(nullptr, returnHint.c_str(), nullptr);
            break;
        }
    }
    
    // 绘制取消确认对话框（覆盖在所有内容之上）
    if (mShowCancelConfirm) {
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
        Gfx::Print(dialogX + dialogW/2, dialogY + 150, 50, Gfx::COLOR_TEXT, 
                  lang.Get("download.confirm_cancel"), Gfx::ALIGN_HORIZONTAL);
        Gfx::Print(dialogX + dialogW/2, dialogY + 215, 40, Gfx::COLOR_ALT_TEXT, 
                  lang.Get("download.cancel_warning"), Gfx::ALIGN_HORIZONTAL);
        
        std::string confirmBtn = std::string(" ") + lang.Get("common.confirm");
        std::string cancelBtn = std::string(" ") + lang.Get("common.back");
        DrawBottomBar(confirmBtn.c_str(), nullptr, cancelBtn.c_str());
    }
}

bool DownloadScreen::Update(Input &input) {
    // 如果显示取消确认对话框，只处理确认/取消
    if (mShowCancelConfirm) {
        if (input.data.buttons_d & Input::BUTTON_A) {
            // 确认取消下载
            mDownloadManager.CancelDownload();
            FileLogger::GetInstance().Log("用户取消下载");
            FileLogger::GetInstance().EndLog();
            mShowCancelConfirm = false;
            return false;  // 返回设置页面
        } else if (input.data.buttons_d & Input::BUTTON_B) {
            // 取消操作，继续下载
            mShowCancelConfirm = false;
            return true;
        }
        return true;
    }
    
    // 更新下载管理器
    mDownloadManager.Update();
    
    switch (mScreenState) {
        case STATE_SELECT_REGION:
            if (input.data.buttons_d & Input::BUTTON_UP) {
                mSelectedRegion = static_cast<SystemRegion>((mSelectedRegion - 1 + REGION_COUNT) % REGION_COUNT);
            }
            if (input.data.buttons_d & Input::BUTTON_DOWN) {
                mSelectedRegion = static_cast<SystemRegion>((mSelectedRegion + 1) % REGION_COUNT);
            }
            if (input.data.buttons_d & Input::BUTTON_A) {
                mScreenState = STATE_SELECT_MIRROR;
            }
            if (input.data.buttons_d & Input::BUTTON_B) {
                return false;  // 返回设置页面
            }
            break;
            
        case STATE_SELECT_MIRROR:
            if (input.data.buttons_d & Input::BUTTON_UP) {
                mSelectedMirror = static_cast<MirrorType>((mSelectedMirror - 1 + MIRROR_COUNT) % MIRROR_COUNT);
                WHBLogPrintf("镜像切换到: %d (%s)", mSelectedMirror, GetMirrorTypeName(mSelectedMirror).c_str());
            }
            if (input.data.buttons_d & Input::BUTTON_DOWN) {
                mSelectedMirror = static_cast<MirrorType>((mSelectedMirror + 1) % MIRROR_COUNT);
                WHBLogPrintf("镜像切换到: %d (%s)", mSelectedMirror, GetMirrorTypeName(mSelectedMirror).c_str());
            }
            if (input.data.buttons_d & Input::BUTTON_A) {
                WHBLogPrintf("确认镜像选择: %d (%s)", mSelectedMirror, GetMirrorTypeName(mSelectedMirror).c_str());
                FileLogger::GetInstance().Log("用户选择镜像: %d (%s)", mSelectedMirror, GetMirrorTypeName(mSelectedMirror).c_str());
                mScreenState = STATE_SELECT_LANGUAGE;
            }
            if (input.data.buttons_d & Input::BUTTON_B) {
                mScreenState = STATE_SELECT_REGION;  // 返回上一步
            }
            break;
            
        case STATE_SELECT_LANGUAGE:
            if (input.data.buttons_d & Input::BUTTON_UP) {
                mSelectedLanguage = static_cast<TargetLanguage>((mSelectedLanguage - 1 + LANG_COUNT) % LANG_COUNT);
            }
            if (input.data.buttons_d & Input::BUTTON_DOWN) {
                mSelectedLanguage = static_cast<TargetLanguage>((mSelectedLanguage + 1) % LANG_COUNT);
            }
            if (input.data.buttons_d & Input::BUTTON_A) {
                StartDownload();
            }
            if (input.data.buttons_d & Input::BUTTON_B) {
                mScreenState = STATE_SELECT_MIRROR;  // 返回上一步
            }
            break;
            
        case STATE_DOWNLOADING:
        case STATE_EXTRACTING:
            if (input.data.buttons_d & Input::BUTTON_B) {
                // 显示取消确认对话框
                mShowCancelConfirm = true;
            }
            break;
            
        case STATE_COMPLETE:
            if (input.data.buttons_d & Input::BUTTON_A) {
                // TODO: 跳转到 CopyScreen
                // 需要在 main.cpp 中实现页面切换逻辑
                return false;
            }
            if (input.data.buttons_d & Input::BUTTON_B) {
                return false;  // 返回设置页面
            }
            break;
            
        case STATE_ERROR:
            if (input.data.buttons_d & Input::BUTTON_A) {
                return false;  // 返回设置页面
            }
            break;
    }
    
    return true;  // 继续保持下载页面
}

std::string DownloadScreen::GetRegionName(SystemRegion region) {
    Language& lang = Language::GetInstance();
    switch (region) {
        case REGION_EU: return lang.Get("download.region_eu");
        case REGION_US: return lang.Get("download.region_us");
        case REGION_JP: return lang.Get("download.region_jp");
        default: return "Unknown";
    }
}

std::string DownloadScreen::GetMirrorTypeName(MirrorType type) {
    Language& lang = Language::GetInstance();
    switch (type) {
        case MIRROR_ORIGINAL: return lang.Get("download.mirror_original");
        case MIRROR_ACCELERATED: return lang.Get("download.mirror_accelerated");
        default: return "Unknown";
    }
}

std::string DownloadScreen::GetLanguageName(TargetLanguage lang) {
    Language& langManager = Language::GetInstance();
    switch (lang) {
        case LANG_CHINESE: return langManager.Get("download.lang_chinese");
        default: return "Unknown";
    }
}

std::string DownloadScreen::FormatFileSize(long bytes) {
    if (bytes == 0) return "0 B";
    
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unit = 0;
    double size = bytes;
    
    while (size >= 1024.0 && unit < 3) {
        size /= 1024.0;
        unit++;
    }
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << size << " " << units[unit];
    return ss.str();
}

std::string DownloadScreen::DownloadAndParseSHA256() {
    // 选择镜像基础URL
    std::string baseUrl;
    if (mSelectedMirror == MIRROR_ACCELERATED) {
        baseUrl = MIRROR_ACCELERATED_BASE;
    } else {
        baseUrl = MIRROR_ORIGINAL_BASE;
    }
    
    std::string sha256Url = baseUrl + "SHA256.txt";
    std::string sha256FilePath = std::string(TEMP_DOWNLOAD_DIR) + "SHA256.txt";
    
    FileLogger::GetInstance().Log("开始下载 SHA256.txt: %s", sha256Url.c_str());
    WHBLogPrintf("下载 SHA256.txt: %s", sha256Url.c_str());
    
    // 确保临时目录存在
    mkdir("fs:/vol", 0777);
    mkdir("fs:/vol/external01", 0777);
    mkdir("fs:/vol/external01/wiiu", 0777);
    mkdir("fs:/vol/external01/wiiu/apps", 0777);
    mkdir("fs:/vol/external01/wiiu/apps/LingoBrew", 0777);
    
    // 使用 CURL 下载 SHA256.txt
    FILE* file = fopen(sha256FilePath.c_str(), "wb");
    if (!file) {
        WHBLogPrintf("无法创建 SHA256.txt 文件");
        FileLogger::GetInstance().LogError("无法创建 SHA256.txt 文件");
        return "";
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        fclose(file);
        WHBLogPrintf("无法初始化 CURL");
        FileLogger::GetInstance().LogError("无法初始化 CURL");
        return "";
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, sha256Url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(file);
    
    if (res != CURLE_OK) {
        WHBLogPrintf("下载 SHA256.txt 失败: %s", curl_easy_strerror(res));
        FileLogger::GetInstance().LogError("下载 SHA256.txt 失败: %s", curl_easy_strerror(res));
        remove(sha256FilePath.c_str());
        return "";
    }
    
    FileLogger::GetInstance().Log("SHA256.txt 下载成功");
    
    // 解析 SHA256.txt
    std::string sha256Value = ParseSHA256File(sha256FilePath);
    
    // 删除临时的 SHA256.txt 文件
    remove(sha256FilePath.c_str());
    
    return sha256Value;
}

std::string DownloadScreen::ParseSHA256File(const std::string& filePath) {
    FILE* file = fopen(filePath.c_str(), "r");
    if (!file) {
        WHBLogPrintf("无法打开 SHA256.txt 文件");
        FileLogger::GetInstance().LogError("无法打开 SHA256.txt 文件");
        return "";
    }
    
    // 生成当前要下载的文件名（不含路径）
    std::string regionCode;
    switch (mSelectedRegion) {
        case REGION_EU: regionCode = "EU"; break;
        case REGION_US: regionCode = "US"; break;
        case REGION_JP: regionCode = "JP"; break;
        default: regionCode = "JP"; break;
    }
    
    std::string langCode;
    switch (mSelectedLanguage) {
        case LANG_CHINESE: langCode = "CH"; break;
        default: langCode = "CH"; break;
    }
    
    std::string targetFilename = regionCode + "-" + langCode + "-" + LANG_FILE_VERSION + ".zip";
    
    FileLogger::GetInstance().Log("查找文件的 SHA-256: %s", targetFilename.c_str());
    WHBLogPrintf("查找文件的 SHA-256: %s", targetFilename.c_str());
    
    // 读取文件内容并查找匹配的 SHA-256
    char line[512];
    std::string sha256Value;
    
    while (fgets(line, sizeof(line), file)) {
        std::string lineStr(line);
        
        // 移除换行符
        if (!lineStr.empty() && lineStr.back() == '\n') {
            lineStr.pop_back();
        }
        if (!lineStr.empty() && lineStr.back() == '\r') {
            lineStr.pop_back();
        }
        
        // 格式: XX-XX-X.XX.zip-sha256:哈希值
        // 例如: US-CH-1.04.zip-sha256:e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
        
        std::string searchPattern = targetFilename + "-sha256:";
        size_t pos = lineStr.find(searchPattern);
        
        if (pos != std::string::npos) {
            // 找到匹配的行，提取 SHA-256 值
            sha256Value = lineStr.substr(pos + searchPattern.length());
            
            // 移除可能的空格
            size_t start = sha256Value.find_first_not_of(" \t");
            size_t end = sha256Value.find_last_not_of(" \t");
            
            if (start != std::string::npos && end != std::string::npos) {
                sha256Value = sha256Value.substr(start, end - start + 1);
            }
            
            FileLogger::GetInstance().Log("找到 SHA-256: %s", sha256Value.c_str());
            WHBLogPrintf("找到 SHA-256: %s", sha256Value.c_str());
            break;
        }
    }
    
    fclose(file);
    
    if (sha256Value.empty()) {
        WHBLogPrintf("在 SHA256.txt 中未找到匹配的条目");
        FileLogger::GetInstance().LogError("在 SHA256.txt 中未找到 %s 的 SHA-256", targetFilename.c_str());
    }
    
    return sha256Value;
}

std::string DownloadScreen::FormatSpeed(long bytesPerSec) {
    return FormatFileSize(bytesPerSec) + "/s";
}
