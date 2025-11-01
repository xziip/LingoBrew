#pragma once
#include <string>
#include <map>

enum class LanguageCode {
    CHINESE,      // 简体中文
    ENGLISH,      // English
    JAPANESE,     // 日本語
    
    LANG_MIN = CHINESE,
    LANG_MAX = JAPANESE
};

class Language {
public:
    static Language& GetInstance();
    
    // 设置语言
    void SetLanguage(LanguageCode lang);
    LanguageCode GetCurrentLanguage() const { return mCurrentLanguage; }
    
    // 获取翻译文本
    const char* Get(const char* key) const;
    
    // 获取语言名称
    const char* GetLanguageName(LanguageCode lang) const;
    
private:
    Language();
    ~Language();
    Language(const Language&) = delete;
    Language& operator=(const Language&) = delete;
    
    void LoadLanguage(LanguageCode lang);
    
    LanguageCode mCurrentLanguage;
    std::map<std::string, std::string> mTexts;
};
