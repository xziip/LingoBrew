#include "Config.hpp"
#include <fstream>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

Config::Config() 
    : mLoggingEnabled(false)
    , mVerboseLogging(false)
    , mDebugMenuVisible(false)  // 默认隐藏,按住 +/-/L 显示
    , mDefaultStartPage(0)  // 0 = 主菜单
    , mLanguage(0)  // 0 = 中文
    , mShowCopyFileDetection(true)  // 默认显示文件检测提醒
    , mConfigPath("fs:/vol/external01/wiiu/lingobrew.cfg") {
    Load();
}

Config::~Config() {
    Save();
}

Config& Config::GetInstance() {
    static Config instance;
    return instance;
}

void Config::SetLoggingEnabled(bool enabled) {
    if (mLoggingEnabled != enabled) {
        mLoggingEnabled = enabled;
        Save();
    }
}

void Config::SetVerboseLogging(bool verbose) {
    if (mVerboseLogging != verbose) {
        mVerboseLogging = verbose;
        Save();
    }
}

void Config::SetDebugMenuVisible(bool visible) {
    if (mDebugMenuVisible != visible) {
        mDebugMenuVisible = visible;
        Save();
    }
}

void Config::SetDefaultStartPage(int page) {
    if (mDefaultStartPage != page) {
        mDefaultStartPage = page;
        Save();
    }
}

void Config::SetLanguage(int lang) {
    if (mLanguage != lang) {
        mLanguage = lang;
        Save();
    }
}

void Config::SetShowCopyFileDetection(bool show) {
    if (mShowCopyFileDetection != show) {
        mShowCopyFileDetection = show;
        Save();
    }
}

bool Config::Load() {
    FILE* file = fopen(mConfigPath.c_str(), "r");
    if (!file) {
        // 配置文件不存在,使用默认值
        return false;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "logging=", 8) == 0) {
            mLoggingEnabled = (line[8] == '1');
        } else if (strncmp(line, "verboselog=", 11) == 0) {
            mVerboseLogging = (line[11] == '1');
        } else if (strncmp(line, "debugmenu=", 10) == 0) {
            mDebugMenuVisible = (line[10] == '1');
        } else if (strncmp(line, "defaultpage=", 12) == 0) {
            int page = atoi(&line[12]);
            // 确保值在有效范围内 (0-8: 主菜单到下载页面)
            if (page >= 0 && page <= 8) {
                mDefaultStartPage = page;
            } else {
                mDefaultStartPage = 0;  // 无效值重置为主菜单
            }
        } else if (strncmp(line, "language=", 9) == 0) {
            int lang = atoi(&line[9]);
            if (lang >= 0 && lang <= 2) {
                mLanguage = lang;
            } else {
                mLanguage = 0;  // 无效值重置为中文
            }
        } else if (strncmp(line, "showcopydetect=", 15) == 0) {
            mShowCopyFileDetection = (line[15] == '1');
        }
    }
    
    fclose(file);
    return true;
}

bool Config::Save() {
    // 确保目录存在
    const char* dirPath = "fs:/vol/external01/wiiu";
    struct stat st;
    if (stat(dirPath, &st) != 0) {
        mkdir(dirPath, 0777);
    }
    
    FILE* file = fopen(mConfigPath.c_str(), "w");
    if (!file) {
        return false;
    }
    
    fprintf(file, "logging=%d\n", mLoggingEnabled ? 1 : 0);
    fprintf(file, "verboselog=%d\n", mVerboseLogging ? 1 : 0);
    fprintf(file, "debugmenu=%d\n", mDebugMenuVisible ? 1 : 0);
    fprintf(file, "defaultpage=%d\n", mDefaultStartPage);
    fprintf(file, "language=%d\n", mLanguage);
    fprintf(file, "showcopydetect=%d\n", mShowCopyFileDetection ? 1 : 0);
    
    fclose(file);
    return true;
}

