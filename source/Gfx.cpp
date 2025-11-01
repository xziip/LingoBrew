#include "Gfx.hpp"
#include "utils/SDL_FontCache.h"
#include "utils/Language.hpp"
#include <cstdarg>
#include <map>
#include <cmath>

#include <coreinit/debug.h>
#include <coreinit/memory.h>

#include <fa-solid-900_ttf.h>
#include <chfont_ttf.h>
#include <ter-u32b_bdf.h>

namespace {

    SDL_Window *window = nullptr;

    SDL_Renderer *renderer = nullptr;

    void *fontData = nullptr;

    uint32_t fontSize = 0;

    std::map<int, FC_Font *> fontMap;

    FC_Font *monospaceFont = nullptr;

    TTF_Font *iconFont = nullptr;

    std::map<Uint16, SDL_Texture *> iconCache;

    FC_Font *GetFontForSize(int size) {
        if (fontMap.contains(size)) {
            return fontMap[size];
        }

        FC_Font *font = FC_CreateFont();
        if (!font) {
            return font;
        }

        // 如果是日文模式，尝试使用系统字体
        // 注释掉以避免每次加载卡顿，等待嵌入日文字体后再启用
        /*
        Language& lang = Language::GetInstance();
        bool fontLoaded = false;
        
        if (lang.GetCurrentLanguage() == LanguageCode::JAPANESE) {
            // 尝试加载Wii U系统日文字体
            const char* systemFontPaths[] = {
                "/vol/storage_mlc01/sys/title/0005001b/10042400/content/CafeStd.ttf",
                "/vol/storage_mlc01/sys/title/0005001b/10042300/content/CafeStd.ttf",
                "/vol/storage_mlc01/sys/title/0005001b/10042200/content/CafeStd.ttf",
                "/vol/storage_mlc01/sys/title/0005001b/10042100/content/CafeStd.ttf",
                "fs:/vol/content/CafeStd.ttf"
            };
            
            for (const char* path : systemFontPaths) {
                if (FC_LoadFont(font, renderer, path, size, Gfx::COLOR_BLACK, TTF_STYLE_NORMAL)) {
                    fontLoaded = true;
                    // 调整日文字体的行间距以避免竖向拉长
                    // 负值会减少行间距，使文字看起来更紧凑
                    FC_SetLineSpacing(font, -4);
                    OSReport("Loaded system font from: %s\n", path);
                    break;
                }
            }
        }
        */
        
        // 如果不是日文或系统字体加载失败，使用内置字体
        bool fontLoaded = false;
        if (!fontLoaded) {
            if (!FC_LoadFont_RW(font, renderer, SDL_RWFromMem(fontData, fontSize), 1, size, Gfx::COLOR_BLACK, TTF_STYLE_NORMAL)) {
                FC_FreeFont(font);
                return nullptr;
            }
        }

        fontMap.insert({size, font});
        return font;
    }

    SDL_Texture *LoadIcon(Uint16 icon) {
        if (iconCache.contains(icon)) {
            return iconCache[icon];
        }

        SDL_Surface *iconSurface = TTF_RenderGlyph_Blended(iconFont, icon, Gfx::COLOR_WHITE);
        if (!iconSurface) {
            return nullptr;
        }

        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, iconSurface);
        SDL_FreeSurface(iconSurface);
        if (!texture) {
            return nullptr;
        }

        iconCache.insert({icon, texture});
        return texture;
    }

} // namespace

namespace Gfx {

    bool Init() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            return false;
        }

        window = SDL_CreateWindow("LingoBrew", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
        if (!window) {
            OSReport("SDL_CreateWindow failed\n");
            return false;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer) {
            OSReport("SDL_CreateRenderer failed\n");
            SDL_DestroyWindow(window);
            window = nullptr;
            return false;
        }

        // 设置渲染质量提示
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2"); // 2 = 最佳质量 (anisotropic filtering)

        fontData = const_cast<uint8_t *>(chfont_ttf);
        fontSize = static_cast<uint32_t>(chfont_ttf_size);

        TTF_Init();

        monospaceFont = FC_CreateFont();
        if (!monospaceFont) {
            return false;
        }

        if (!FC_LoadFont_RW(monospaceFont, renderer, SDL_RWFromMem((void *) ter_u32b_bdf, ter_u32b_bdf_size), 1, 32, Gfx::COLOR_BLACK, TTF_STYLE_NORMAL)) {
            FC_FreeFont(monospaceFont);
            return false;
        }

        // icons @256 should be large enough for our needs
        iconFont = TTF_OpenFontRW(SDL_RWFromMem((void *) fa_solid_900_ttf, fa_solid_900_ttf_size), 1, 256);
        if (!iconFont) {
            return false;
        }

        return true;
    }

    void Shutdown() {
        for (const auto &[key, value] : fontMap) {
            FC_FreeFont(value);
        }

        for (const auto &[key, value] : iconCache) {
            SDL_DestroyTexture(value);
        }

        FC_FreeFont(monospaceFont);
        TTF_CloseFont(iconFont);
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
    
    void ReloadFonts() {
        // 清除现有字体缓存
        for (const auto &[key, value] : fontMap) {
            FC_FreeFont(value);
        }
        fontMap.clear();
        
        OSReport("Font cache cleared for language change\n");
    }

    void Clear(SDL_Color color) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderClear(renderer);
    }

    void Render() {
        SDL_RenderPresent(renderer);
    }

    void DrawRectFilled(int x, int y, int w, int h, SDL_Color color) {
        SDL_Rect rect{x, y, w, h};
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
    }

    void DrawRect(int x, int y, int w, int h, int borderSize, SDL_Color color) {
        DrawRectFilled(x, y, w, borderSize, color);
        DrawRectFilled(x, y + h - borderSize, w, borderSize, color);
        DrawRectFilled(x, y, borderSize, h, color);
        DrawRectFilled(x + w - borderSize, y, borderSize, h, color);
    }

    void DrawIcon(int x, int y, int size, SDL_Color color, Uint16 icon, AlignFlags align, double angle) {
        SDL_Texture *iconTex = LoadIcon(icon);
        if (!iconTex) {
            return;
        }

        SDL_SetTextureColorMod(iconTex, color.r, color.g, color.b);
        SDL_SetTextureAlphaMod(iconTex, color.a);

        int w, h;
        SDL_QueryTexture(iconTex, nullptr, nullptr, &w, &h);

        SDL_Rect rect;
        rect.x = x;
        rect.y = y;
        // scale the width based on hight to keep AR
        rect.w = (int) (((float) w / h) * size);
        rect.h = size;

        if (align & ALIGN_RIGHT) {
            rect.x -= rect.w;
        } else if (align & ALIGN_HORIZONTAL) {
            rect.x -= rect.w / 2;
        }

        if (align & ALIGN_BOTTOM) {
            rect.y -= rect.h;
        } else if (align & ALIGN_VERTICAL) {
            rect.y -= rect.h / 2;
        }

        // draw the icon
        if (angle) {
            SDL_RenderCopyEx(renderer, iconTex, nullptr, &rect, angle, nullptr, SDL_FLIP_NONE);
        } else {
            SDL_RenderCopy(renderer, iconTex, nullptr, &rect);
        }
    }

    int GetIconWidth(int size, Uint16 icon) {
        SDL_Texture *iconTex = LoadIcon(icon);
        if (!iconTex) {
            return 0;
        }

        int w, h;
        SDL_QueryTexture(iconTex, nullptr, nullptr, &w, &h);

        return (int) (((float) w / h) * size);
    }

    void Print(int x, int y, int size, SDL_Color color, std::string text, AlignFlags align, bool monospace) {
        FC_Font *font = monospace ? monospaceFont : GetFontForSize(size);
        if (!font) {
            return;
        }

        FC_Effect effect;
        effect.color = color;

        // scale monospace font based on size
        if (monospace) {
            effect.scale = FC_MakeScale(size / 28.0f, size / 28.0f);
            // TODO figure out how to center this properly
            y += 5;
        } else {
            effect.scale = FC_MakeScale(1, 1);
        }

        if (align & ALIGN_LEFT) {
            effect.alignment = FC_ALIGN_LEFT;
        } else if (align & ALIGN_RIGHT) {
            effect.alignment = FC_ALIGN_RIGHT;
        } else if (align & ALIGN_HORIZONTAL) {
            effect.alignment = FC_ALIGN_CENTER;
        } else {
            // left by default
            effect.alignment = FC_ALIGN_LEFT;
        }

        if (align & ALIGN_BOTTOM) {
            y -= GetTextHeight(size, text, monospace);
        } else if (align & ALIGN_VERTICAL) {
            y -= GetTextHeight(size, text, monospace) / 2;
        }

        FC_DrawEffect(font, renderer, x, y, effect, "%s", text.c_str());
    }

    int GetTextWidth(int size, std::string text, bool monospace) {
        FC_Font *font = monospace ? monospaceFont : GetFontForSize(size);
        if (!font) {
            return 0;
        }

        float scale = monospace ? (size / 28.0f) : 1.0f;

        return FC_GetWidth(font, "%s", text.c_str()) * scale;
    }

    int GetTextHeight(int size, std::string text, bool monospace) {
        // TODO this doesn't work nicely with monospace yet
        monospace = false;

        FC_Font *font = monospace ? monospaceFont : GetFontForSize(size);
        if (!font) {
            return 0;
        }

        float scale = monospace ? (size / 28.0f) : 1.0f;

        return FC_GetHeight(GetFontForSize(size), "%s", text.c_str()) * scale;
    }

    SDL_Color GetRainbowColor(float speed) {
        // 使用SDL的毫秒计时器
        uint32_t timeMs = SDL_GetTicks();
        
        // 定义几个高对比度的鲜艳颜色（避免深蓝等不清晰的颜色）
        const SDL_Color colors[] = {
            {0xff, 0x33, 0x33, 0xff},  // 红色
            {0xff, 0xaa, 0x00, 0xff},  // 橙色
            {0xff, 0xff, 0x00, 0xff},  // 黄色
            {0x00, 0xff, 0x00, 0xff},  // 绿色
            {0x00, 0xff, 0xff, 0xff},  // 青色
            {0xff, 0x00, 0xff, 0xff},  // 洋红
        };
        
        const int numColors = sizeof(colors) / sizeof(colors[0]);
        
        // 根据时间和速度计算当前位置（带小数）
        float position = timeMs * speed * 0.001f;
        int colorIndex1 = ((int)position) % numColors;
        int colorIndex2 = (colorIndex1 + 1) % numColors;
        
        // 计算两个颜色之间的插值比例
        float blend = fmod(position, 1.0f);
        
        // 线性插值两个颜色
        SDL_Color result;
        result.r = (uint8_t)(colors[colorIndex1].r * (1.0f - blend) + colors[colorIndex2].r * blend);
        result.g = (uint8_t)(colors[colorIndex1].g * (1.0f - blend) + colors[colorIndex2].g * blend);
        result.b = (uint8_t)(colors[colorIndex1].b * (1.0f - blend) + colors[colorIndex2].b * blend);
        result.a = 0xff;
        
        return result;
    }

} // namespace Gfx
