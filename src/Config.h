#pragma once
#include <windows.h>
#include <string>

namespace Config {
    // Window Settings
    inline const wchar_t* WINDOW_CLASS_NAME = L"RinganBrowserMainWndClass";
    inline const wchar_t* APP_TITLE = L"RinganBrowser";
    inline const int DEFAULT_WIDTH = 1200;
    inline const int DEFAULT_HEIGHT = 800;
    inline const int MIN_WIDTH = 640;
    inline const int MIN_HEIGHT = 480;

    // Dimensions
    inline const int TAB_BAR_HEIGHT = 38;
    inline const int NAV_BAR_HEIGHT = 44;
    inline const int TOP_BAR_HEIGHT = TAB_BAR_HEIGHT + NAV_BAR_HEIGHT; // 82px
    inline const int TAB_MIN_WIDTH = 120;
    inline const int TAB_MAX_WIDTH = 220;
    inline const int TAB_NEW_BTN_WIDTH = 34;
    inline const int NAV_BTN_SIZE = 34;

    // Dark Theme Colors (BGR for Win32 COLORREF)
    inline const COLORREF COLOR_TITLEBAR_BG = RGB(15, 23, 42);      // Slate 900
    inline const COLORREF COLOR_NAVBAR_BG   = RGB(30, 41, 59);      // Slate 800
    inline const COLORREF COLOR_TAB_ACTIVE   = RGB(30, 41, 59);      // Matches navbar
    inline const COLORREF COLOR_TAB_INACTIVE = RGB(15, 23, 42);      // Matches titlebar
    inline const COLORREF COLOR_TAB_HOVER    = RGB(51, 65, 85);      // Slate 700
    inline const COLORREF COLOR_BORDER       = RGB(51, 65, 85);      // Slate 700
    inline const COLORREF COLOR_TEXT_MAIN    = RGB(248, 250, 252);   // Slate 50
    inline const COLORREF COLOR_TEXT_MUTED   = RGB(148, 163, 184);   // Slate 400
    inline const COLORREF COLOR_ACCENT       = RGB(99, 102, 241);    // Indigo 500
    inline const COLORREF COLOR_ACCENT_CYAN  = RGB(6, 182, 212);     // Cyan 500
    inline const COLORREF COLOR_URL_BG       = RGB(15, 23, 42);      // Slate 900
    inline const COLORREF COLOR_URL_BORDER   = RGB(71, 85, 105);     // Slate 600
    inline const COLORREF COLOR_URL_FOCUS    = RGB(99, 102, 241);    // Indigo
    inline const COLORREF COLOR_BTN_HOVER    = RGB(51, 65, 85);      // Slate 700

    // Search Engine URL templates
    inline const wchar_t* SEARCH_ENGINE_DUCKDUCKGO = L"https://duckduckgo.com/?q=";
    inline const wchar_t* SEARCH_ENGINE_GOOGLE     = L"https://www.google.com/search?q=";

    // Helper to get executable path
    inline std::wstring GetExecutableDir() {
        wchar_t buffer[MAX_PATH];
        GetModuleFileNameW(NULL, buffer, MAX_PATH);
        std::wstring path(buffer);
        size_t pos = path.find_last_of(L"\\/");
        return (pos != std::wstring::npos) ? path.substr(0, pos) : L"";
    }

    // Helper to get local startpage file URL
    inline std::wstring GetStartPageUrl() {
        std::wstring exeDir = GetExecutableDir();
        std::wstring path = exeDir + L"\\assets\\startpage.html";
        // Convert to file URI
        std::wstring uri = L"file:///";
        for (wchar_t c : path) {
            if (c == L'\\') uri += L'/';
            else uri += c;
        }
        return uri;
    }
}
