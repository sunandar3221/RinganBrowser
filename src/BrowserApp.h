#pragma once
#include <windows.h>
#include <commctrl.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <memory>
#include "Config.h"
#include "VectorIcons.h"
#include "WebViewHandlers.h"

struct TabItem {
    std::wstring id;
    std::wstring title;
    std::wstring url;
    bool isLoading = false;
    bool isSecure = true;
    double zoomFactor = 1.0;
    ICoreWebView2Controller* controller = nullptr;
    ICoreWebView2* webview = nullptr;
    EventRegistrationToken tokenTitle;
    EventRegistrationToken tokenSource;
    EventRegistrationToken tokenNavStart;
    EventRegistrationToken tokenNavComplete;
    EventRegistrationToken tokenHistory;
    EventRegistrationToken tokenNewWindow;
    EventRegistrationToken tokenWebMessage;
};

enum class HoverArea {
    None,
    BtnBack,
    BtnForward,
    BtnReload,
    BtnHome,
    BtnBookmark,
    BtnDownload,
    BtnZoom,
    BtnDevTools,
    BtnSettings,
    BtnNewTab,
    TabItem,
    TabClose
};

class BrowserApp {
public:
    BrowserApp();
    ~BrowserApp();

    bool Initialize(HINSTANCE hInstance, int nCmdShow);
    LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // Navigation
    void Navigate(const std::wstring& url);
    void GoBack();
    void GoForward();
    void Reload();
    void Stop();
    void GoHome();
    void OpenDownloadsFolder();
    void ToggleDevTools();
    void ZoomIn();
    void ZoomOut();
    void ZoomReset();
    void ShowSettingsMenu();
    void ShowAboutDialog();
    std::wstring GetSearchEngineUrl() const;
    void SetSearchEngine(const std::wstring& engine);
    void SyncSearchEngineToAllTabs();

    // Tabs
    void NewTab(const std::wstring& initialUrl = L"");
    void CloseTab(int index);
    void SwitchTab(int index);

private:
    void InitWebViewEnvironment();
    void CreateWebViewForTab(int tabIndex, const std::wstring& urlToLoad);
    void ResizeViews();
    void UpdateTitleAndUrlBar();
    void PaintChrome(HDC hdc);
    void CheckHover(int x, int y);
    void HandleClick(int x, int y);

    // Helper hit testing
    RECT GetTabRect(int index) const;
    RECT GetTabCloseRect(int index) const;
    RECT GetNewTabBtnRect() const;
    RECT GetNavBtnRect(int btnIndex) const;
    RECT GetUrlBarRect() const;

    static LRESULT CALLBACK UrlEditSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

    void LoadSearchEnginePreference();
    void SaveSearchEnginePreference();

    HINSTANCE m_hInstance = nullptr;
    HWND m_hWnd = nullptr;
    HWND m_hUrlEdit = nullptr;
    HFONT m_hFontUI = nullptr;
    HFONT m_hFontBold = nullptr;

    ICoreWebView2Environment* m_pEnvironment = nullptr;
    std::vector<TabItem> m_tabs;
    int m_activeTabIndex = -1;
    bool m_envInitialized = false;

    // Search Engine Preference (google, bing, duckduckgo, wikipedia)
    std::wstring m_searchEngine = L"google";

    // Hover state
    HoverArea m_hoverArea = HoverArea::None;
    int m_hoverIndex = -1; // tab index if TabItem or TabClose
    bool m_isBookmarked = false;

    // Window size cache
    int m_clientW = 0;
    int m_clientH = 0;
};
