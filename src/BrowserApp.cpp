#include "BrowserApp.h"
#include <windowsx.h>
#include <shlobj.h>
#include <algorithm>
#include <sstream>

using namespace Gdiplus;

// Linker pragma or manual import for CreateCoreWebView2EnvironmentWithOptions
extern "C" HRESULT STDAPICALLTYPE CreateCoreWebView2EnvironmentWithOptions(
    PCWSTR browserExecutableFolder,
    PCWSTR userDataFolder,
    ICoreWebView2EnvironmentOptions* environmentOptions,
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* environmentCreatedHandler);

BrowserApp::BrowserApp() {
    LoadSearchEnginePreference();
}

BrowserApp::~BrowserApp() {
    for (auto& tab : m_tabs) {
        if (tab.webview) {
            tab.webview->Release();
            tab.webview = nullptr;
        }
        if (tab.controller) {
            tab.controller->Close();
            tab.controller->Release();
            tab.controller = nullptr;
        }
    }
    if (m_pEnvironment) {
        m_pEnvironment->Release();
        m_pEnvironment = nullptr;
    }
    if (m_hFontUI) DeleteObject(m_hFontUI);
    if (m_hFontBold) DeleteObject(m_hFontBold);
}

bool BrowserApp::Initialize(HINSTANCE hInstance, int nCmdShow) {
    m_hInstance = hInstance;

    // Create fonts
    m_hFontUI = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    m_hFontBold = CreateFontW(-13, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    // Register Window Class
    WNDCLASSEXW wcex = { sizeof(WNDCLASSEXW) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
        BrowserApp* pApp = nullptr;
        if (msg == WM_NCCREATE) {
            auto cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            pApp = reinterpret_cast<BrowserApp*>(cs->lpCreateParams);
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pApp));
            pApp->m_hWnd = hWnd;
            return DefWindowProcW(hWnd, msg, wParam, lParam);
        } else {
            pApp = reinterpret_cast<BrowserApp*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        }

        if (pApp) {
            return pApp->HandleMessage(hWnd, msg, wParam, lParam);
        }
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    };
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(101));
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszClassName = Config::WINDOW_CLASS_NAME;
    wcex.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCEW(101));

    if (!RegisterClassExW(&wcex)) {
        return false;
    }

    // Calculate initial window position (centered)
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenW - Config::DEFAULT_WIDTH) / 2;
    int posY = (screenH - Config::DEFAULT_HEIGHT) / 2;

    m_hWnd = CreateWindowExW(
        0,
        Config::WINDOW_CLASS_NAME,
        Config::APP_TITLE,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        posX, posY,
        Config::DEFAULT_WIDTH, Config::DEFAULT_HEIGHT,
        nullptr, nullptr, hInstance, this
    );

    if (!m_hWnd) return false;

    // Create URL Edit Control
    m_hUrlEdit = CreateWindowExW(
        0, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_LEFT,
        0, 0, 0, 0,
        m_hWnd, (HMENU)1001, hInstance, nullptr
    );

    SendMessageW(m_hUrlEdit, WM_SETFONT, (WPARAM)m_hFontUI, TRUE);

    // Subclass URL edit control to handle Enter, Esc, Focus
    SetWindowSubclass(m_hUrlEdit, UrlEditSubclassProc, 1, (DWORD_PTR)this);

    int showCmd = (nCmdShow <= 0 || nCmdShow == SW_HIDE) ? SW_SHOWNORMAL : nCmdShow;
    ShowWindow(m_hWnd, showCmd);
    UpdateWindow(m_hWnd);
    SetForegroundWindow(m_hWnd);

    // Initialize WebView2 Environment & First Tab
    InitWebViewEnvironment();

    return true;
}

LRESULT CALLBACK BrowserApp::UrlEditSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    BrowserApp* app = (BrowserApp*)dwRefData;
    switch (uMsg) {
        case WM_KEYDOWN: {
            if (wParam == VK_RETURN) {
                wchar_t buf[2048] = { 0 };
                GetWindowTextW(hWnd, buf, 2048);
                app->Navigate(buf);
                return 0;
            } else if (wParam == VK_ESCAPE) {
                app->UpdateTitleAndUrlBar();
                return 0;
            }
            break;
        }
        case WM_SETFOCUS: {
            SendMessageW(hWnd, EM_SETSEL, 0, -1);
            break;
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void BrowserApp::InitWebViewEnvironment() {
    wchar_t localAppData[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData);
    std::wstring userDataDir = std::wstring(localAppData) + L"\\RinganBrowser\\UserData";

    auto handler = new WebViewHandlers::EnvironmentHandler(
        [this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
            if (SUCCEEDED(result) && env) {
                m_pEnvironment = env;
                m_pEnvironment->AddRef();
                m_envInitialized = true;

                // Create initial tab loading start page
                NewTab(Config::GetStartPageUrl());
            } else {
                MessageBoxW(m_hWnd, 
                    L"Gagal menginisialisasi WebView2 Runtime.\nPastikan Microsoft Edge WebView2 Runtime terpasang di sistem.", 
                    L"RinganBrowser Error", MB_ICONERROR | MB_OK);
            }
            return S_OK;
        }
    );

    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataDir.c_str(), nullptr, handler);
    if (FAILED(hr)) {
        handler->Release();
    }
}

void BrowserApp::NewTab(const std::wstring& initialUrl) {
    std::wstring targetUrl = initialUrl.empty() ? Config::GetStartPageUrl() : initialUrl;

    TabItem tab;
    tab.id = L"tab_" + std::to_wstring(m_tabs.size() + 1);
    tab.title = L"Tab Baru";
    tab.url = targetUrl;
    tab.isLoading = true;

    m_tabs.push_back(tab);
    int newIndex = (int)m_tabs.size() - 1;

    if (m_envInitialized && m_pEnvironment) {
        CreateWebViewForTab(newIndex, targetUrl);
    }
    SwitchTab(newIndex);
}

void BrowserApp::CreateWebViewForTab(int tabIndex, const std::wstring& urlToLoad) {
    if (!m_pEnvironment || tabIndex < 0 || tabIndex >= (int)m_tabs.size()) return;

    auto handler = new WebViewHandlers::ControllerHandler(
        [this, tabIndex, urlToLoad](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
            if (SUCCEEDED(result) && controller) {
                TabItem& tab = m_tabs[tabIndex];
                tab.controller = controller;
                tab.controller->AddRef();

                tab.controller->get_CoreWebView2(&tab.webview);
                if (tab.webview) {
                    // Document Title Changed
                    auto titleHandler = new WebViewHandlers::DocumentTitleChangedHandler(
                        [this, tabIndex](ICoreWebView2* sender, IUnknown*) -> HRESULT {
                            if (tabIndex < (int)m_tabs.size() && m_tabs[tabIndex].webview == sender) {
                                LPWSTR title = nullptr;
                                if (SUCCEEDED(sender->get_DocumentTitle(&title)) && title) {
                                    m_tabs[tabIndex].title = title;
                                    CoTaskMemFree(title);
                                }
                                UpdateTitleAndUrlBar();
                                InvalidateRect(m_hWnd, nullptr, FALSE);
                            }
                            return S_OK;
                        }
                    );
                    tab.webview->add_DocumentTitleChanged(titleHandler, &tab.tokenTitle);
                    titleHandler->Release();

                    // Source Changed (URL Changed)
                    auto sourceHandler = new WebViewHandlers::SourceChangedHandler(
                        [this, tabIndex](ICoreWebView2* sender, ICoreWebView2SourceChangedEventArgs*) -> HRESULT {
                            if (tabIndex < (int)m_tabs.size() && m_tabs[tabIndex].webview == sender) {
                                LPWSTR uri = nullptr;
                                if (SUCCEEDED(sender->get_Source(&uri)) && uri) {
                                    m_tabs[tabIndex].url = uri;
                                    m_tabs[tabIndex].isSecure = (wcsstr(uri, L"https://") != nullptr);
                                    CoTaskMemFree(uri);
                                }
                                UpdateTitleAndUrlBar();
                                InvalidateRect(m_hWnd, nullptr, FALSE);
                            }
                            return S_OK;
                        }
                    );
                    tab.webview->add_SourceChanged(sourceHandler, &tab.tokenSource);
                    sourceHandler->Release();

                    // Navigation Starting
                    auto navStartHandler = new WebViewHandlers::NavigationStartingHandler(
                        [this, tabIndex](ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs*) -> HRESULT {
                            if (tabIndex < (int)m_tabs.size() && m_tabs[tabIndex].webview == sender) {
                                m_tabs[tabIndex].isLoading = true;
                                InvalidateRect(m_hWnd, nullptr, FALSE);
                            }
                            return S_OK;
                        }
                    );
                    tab.webview->add_NavigationStarting(navStartHandler, &tab.tokenNavStart);
                    navStartHandler->Release();

                    // Navigation Completed
                    auto navCompHandler = new WebViewHandlers::NavigationCompletedHandler(
                        [this, tabIndex](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs*) -> HRESULT {
                            if (tabIndex < (int)m_tabs.size() && m_tabs[tabIndex].webview == sender) {
                                m_tabs[tabIndex].isLoading = false;
                                InvalidateRect(m_hWnd, nullptr, FALSE);

                                // If this is start page, sync the active search engine badge
                                if (m_tabs[tabIndex].url.find(L"startpage.html") != std::wstring::npos) {
                                    std::wstring js = L"if (typeof updateSearchEngineBadge === 'function') { updateSearchEngineBadge('" + m_searchEngine + L"'); }";
                                    sender->ExecuteScript(js.c_str(), nullptr);
                                }
                            }
                            return S_OK;
                        }
                    );
                    tab.webview->add_NavigationCompleted(navCompHandler, &tab.tokenNavComplete);
                    navCompHandler->Release();

                    // Web Message Received (from startpage.html)
                    auto msgHandler = new WebViewHandlers::WebMessageReceivedHandler(
                        [this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                            LPWSTR jsonMsg = nullptr;
                            if (SUCCEEDED(args->get_WebMessageAsJson(&jsonMsg)) && jsonMsg) {
                                std::wstring str(jsonMsg);
                                CoTaskMemFree(jsonMsg);
                                if (str.find(L"\"setSearchEngine\"") != std::wstring::npos) {
                                    if (str.find(L"\"google\"") != std::wstring::npos) SetSearchEngine(L"google");
                                    else if (str.find(L"\"bing\"") != std::wstring::npos) SetSearchEngine(L"bing");
                                    else if (str.find(L"\"duckduckgo\"") != std::wstring::npos) SetSearchEngine(L"duckduckgo");
                                    else if (str.find(L"\"wikipedia\"") != std::wstring::npos) SetSearchEngine(L"wikipedia");
                                }
                            }
                            return S_OK;
                        }
                    );
                    tab.webview->add_WebMessageReceived(msgHandler, &tab.tokenWebMessage);
                    msgHandler->Release();

                    // New Window Requested -> Open in new tab!
                    auto newWinHandler = new WebViewHandlers::NewWindowRequestedHandler(
                        [this](ICoreWebView2* sender, ICoreWebView2NewWindowRequestedEventArgs* args) -> HRESULT {
                            args->put_Handled(TRUE);
                            LPWSTR uri = nullptr;
                            if (SUCCEEDED(args->get_Uri(&uri)) && uri) {
                                NewTab(uri);
                                CoTaskMemFree(uri);
                            }
                            return S_OK;
                        }
                    );
                    tab.webview->add_NewWindowRequested(newWinHandler, &tab.tokenNewWindow);
                    newWinHandler->Release();

                    // Navigate to initial URL
                    tab.webview->Navigate(urlToLoad.c_str());
                }

                // Make visible only if it is the active tab
                BOOL isVis = (tabIndex == m_activeTabIndex);
                tab.controller->put_IsVisible(isVis);
                ResizeViews();
            }
            return S_OK;
        }
    );

    m_pEnvironment->CreateCoreWebView2Controller(m_hWnd, handler);
    handler->Release();
}

void BrowserApp::CloseTab(int index) {
    if (index < 0 || index >= (int)m_tabs.size()) return;

    if (m_tabs.size() == 1) {
        // If closing the only tab, navigate to startpage instead of terminating
        Navigate(Config::GetStartPageUrl());
        return;
    }

    TabItem& tab = m_tabs[index];
    if (tab.webview) {
        tab.webview->Release();
        tab.webview = nullptr;
    }
    if (tab.controller) {
        tab.controller->Close();
        tab.controller->Release();
        tab.controller = nullptr;
    }

    m_tabs.erase(m_tabs.begin() + index);

    if (m_activeTabIndex >= (int)m_tabs.size()) {
        m_activeTabIndex = (int)m_tabs.size() - 1;
    } else if (m_activeTabIndex > index) {
        m_activeTabIndex--;
    }

    SwitchTab(m_activeTabIndex);
    InvalidateRect(m_hWnd, nullptr, FALSE);
}

void BrowserApp::SwitchTab(int index) {
    if (index < 0 || index >= (int)m_tabs.size()) return;

    m_activeTabIndex = index;

    for (size_t i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs[i].controller) {
            m_tabs[i].controller->put_IsVisible((int)i == m_activeTabIndex);
        }
    }

    UpdateTitleAndUrlBar();
    ResizeViews();
    InvalidateRect(m_hWnd, nullptr, FALSE);
}

void BrowserApp::Navigate(const std::wstring& input) {
    if (m_activeTabIndex < 0 || m_activeTabIndex >= (int)m_tabs.size()) return;
    TabItem& tab = m_tabs[m_activeTabIndex];

    std::wstring trimmed = input;
    trimmed.erase(0, trimmed.find_first_not_of(L" \t\r\n"));
    trimmed.erase(trimmed.find_last_not_of(L" \t\r\n") + 1);

    if (trimmed.empty()) {
        trimmed = Config::GetStartPageUrl();
    }

    std::wstring finalUrl;
    // Check if input is a valid protocol or local file
    if (trimmed.rfind(L"http://", 0) == 0 ||
        trimmed.rfind(L"https://", 0) == 0 ||
        trimmed.rfind(L"file:///", 0) == 0 ||
        trimmed.rfind(L"edge://", 0) == 0 ||
        trimmed.rfind(L"about:", 0) == 0) {
        finalUrl = trimmed;
    } else if (trimmed.find(L".") != std::wstring::npos && trimmed.find(L" ") == std::wstring::npos) {
        // Looks like domain name e.g. "github.com" or "wikipedia.org"
        finalUrl = L"https://" + trimmed;
    } else {
        // Query search using active search engine
        finalUrl = GetSearchEngineUrl() + trimmed;
    }

    if (tab.webview) {
        tab.webview->Navigate(finalUrl.c_str());
    } else {
        tab.url = finalUrl;
    }
}

void BrowserApp::GoBack() {
    if (m_activeTabIndex >= 0 && m_activeTabIndex < (int)m_tabs.size()) {
        auto& tab = m_tabs[m_activeTabIndex];
        if (tab.webview) tab.webview->GoBack();
    }
}

void BrowserApp::GoForward() {
    if (m_activeTabIndex >= 0 && m_activeTabIndex < (int)m_tabs.size()) {
        auto& tab = m_tabs[m_activeTabIndex];
        if (tab.webview) tab.webview->GoForward();
    }
}

void BrowserApp::Reload() {
    if (m_activeTabIndex >= 0 && m_activeTabIndex < (int)m_tabs.size()) {
        auto& tab = m_tabs[m_activeTabIndex];
        if (tab.webview) {
            if (tab.isLoading) tab.webview->Stop();
            else tab.webview->Reload();
        }
    }
}

void BrowserApp::Stop() {
    if (m_activeTabIndex >= 0 && m_activeTabIndex < (int)m_tabs.size()) {
        auto& tab = m_tabs[m_activeTabIndex];
        if (tab.webview) tab.webview->Stop();
    }
}

void BrowserApp::GoHome() {
    Navigate(Config::GetStartPageUrl());
}

void BrowserApp::ToggleDevTools() {
    if (m_activeTabIndex >= 0 && m_activeTabIndex < (int)m_tabs.size()) {
        auto& tab = m_tabs[m_activeTabIndex];
        if (tab.webview) tab.webview->OpenDevToolsWindow();
    }
}

void BrowserApp::ZoomIn() {
    if (m_activeTabIndex >= 0 && m_activeTabIndex < (int)m_tabs.size()) {
        auto& tab = m_tabs[m_activeTabIndex];
        if (tab.controller) {
            tab.zoomFactor = std::min(3.0, tab.zoomFactor + 0.1);
            tab.controller->put_ZoomFactor(tab.zoomFactor);
        }
    }
}

void BrowserApp::ZoomOut() {
    if (m_activeTabIndex >= 0 && m_activeTabIndex < (int)m_tabs.size()) {
        auto& tab = m_tabs[m_activeTabIndex];
        if (tab.controller) {
            tab.zoomFactor = std::max(0.3, tab.zoomFactor - 0.1);
            tab.controller->put_ZoomFactor(tab.zoomFactor);
        }
    }
}

void BrowserApp::ZoomReset() {
    if (m_activeTabIndex >= 0 && m_activeTabIndex < (int)m_tabs.size()) {
        auto& tab = m_tabs[m_activeTabIndex];
        if (tab.controller) {
            tab.zoomFactor = 1.0;
            tab.controller->put_ZoomFactor(1.0);
        }
    }
}

void BrowserApp::OpenDownloadsFolder() {
    wchar_t downloadPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, downloadPath))) {
        std::wstring fullPath = std::wstring(downloadPath) + L"\\Downloads";
        ShellExecuteW(m_hWnd, L"open", fullPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
}

void BrowserApp::LoadSearchEnginePreference() {
    wchar_t localAppData[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData))) {
        std::wstring iniFile = std::wstring(localAppData) + L"\\RinganBrowser\\settings.ini";
        wchar_t buf[64] = { 0 };
        GetPrivateProfileStringW(L"Preferences", L"SearchEngine", L"google", buf, 64, iniFile.c_str());
        m_searchEngine = buf;
    }
    if (m_searchEngine.empty()) m_searchEngine = L"google";
}

void BrowserApp::SaveSearchEnginePreference() {
    wchar_t localAppData[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData))) {
        std::wstring dir = std::wstring(localAppData) + L"\\RinganBrowser";
        CreateDirectoryW(dir.c_str(), NULL);
        std::wstring iniFile = dir + L"\\settings.ini";
        WritePrivateProfileStringW(L"Preferences", L"SearchEngine", m_searchEngine.c_str(), iniFile.c_str());
    }
}

std::wstring BrowserApp::GetSearchEngineUrl() const {
    if (m_searchEngine == L"bing") return Config::SEARCH_ENGINE_BING;
    if (m_searchEngine == L"duckduckgo") return Config::SEARCH_ENGINE_DUCKDUCKGO;
    if (m_searchEngine == L"wikipedia") return Config::SEARCH_ENGINE_WIKIPEDIA;
    return Config::SEARCH_ENGINE_GOOGLE;
}

void BrowserApp::SetSearchEngine(const std::wstring& engine) {
    m_searchEngine = engine;
    SaveSearchEnginePreference();
    SyncSearchEngineToAllTabs();
}

void BrowserApp::SyncSearchEngineToAllTabs() {
    std::wstring js = L"if (typeof updateSearchEngineBadge === 'function') { updateSearchEngineBadge('" + m_searchEngine + L"'); }";
    for (auto& tab : m_tabs) {
        if (tab.webview) {
            tab.webview->ExecuteScript(js.c_str(), nullptr);
        }
    }
}

void BrowserApp::ShowSettingsMenu() {
    HMENU hMenu = CreatePopupMenu();
    HMENU hEngineSub = CreatePopupMenu();

    AppendMenuW(hEngineSub, (m_searchEngine == L"google" ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 2001, L"Google (Default)");
    AppendMenuW(hEngineSub, (m_searchEngine == L"bing" ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 2002, L"Bing");
    AppendMenuW(hEngineSub, (m_searchEngine == L"duckduckgo" ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 2003, L"DuckDuckGo");
    AppendMenuW(hEngineSub, (m_searchEngine == L"wikipedia" ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 2004, L"Wikipedia");

    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hEngineSub, L"Mesin Pencari Default");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, 2010, L"Tentang RinganBrowser...");

    RECT btnRc = GetNavBtnRect(8);
    POINT pt = { btnRc.left, btnRc.bottom + 2 };
    ClientToScreen(m_hWnd, &pt);

    int cmd = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hWnd, NULL);

    DestroyMenu(hEngineSub);
    DestroyMenu(hMenu);

    if (cmd == 2001) SetSearchEngine(L"google");
    else if (cmd == 2002) SetSearchEngine(L"bing");
    else if (cmd == 2003) SetSearchEngine(L"duckduckgo");
    else if (cmd == 2004) SetSearchEngine(L"wikipedia");
    else if (cmd == 2010) ShowAboutDialog();
}

void BrowserApp::ShowAboutDialog() {
    std::wstring curName = L"Google";
    if (m_searchEngine == L"bing") curName = L"Bing";
    else if (m_searchEngine == L"duckduckgo") curName = L"DuckDuckGo";
    else if (m_searchEngine == L"wikipedia") curName = L"Wikipedia";

    std::wstring msg =
        L"RinganBrowser v1.0.0 (Release)\n"
        L"--------------------------------------------------\n"
        L"Browser Modern, Cepat & Ultra Ringan\n"
        L"Ditulis murni dengan C++20 Win32 API.\n\n"
        L"Mesin Pencari Aktif: " + curName + L"\n\n"
        L"Keunggulan Utama:\n"
        L"• Tanpa fork Chromium (build super cepat < 5 detik)\n"
        L"• Ukuran executable sangat kecil (~800 KB)\n"
        L"• UI Modern dengan Ikon Vektor & SVG murni\n"
        L"• Dukungan penuh standar web modern (HTML5, CSS3, JS ES2024, SVG, WebGL)\n"
        L"• Multi-Tab Browsing & DevTools Terintegrasi (F12)\n\n"
        L"Pintasan Keyboard:\n"
        L"• Ctrl+T : Buka Tab Baru\n"
        L"• Ctrl+W : Tutup Tab\n"
        L"• Ctrl+L / Alt+D : Fokus Bar Alamat\n"
        L"• Ctrl+R / F5 : Muat Ulang Halaman\n"
        L"• Ctrl+J : Folder Unduhan\n"
        L"• Alt+Left / Right : Mundur / Maju\n"
        L"• F12 : Buka Developer Tools";

    MessageBoxW(m_hWnd, msg.c_str(), L"Tentang RinganBrowser", MB_ICONINFORMATION | MB_OK);
}

void BrowserApp::UpdateTitleAndUrlBar() {
    if (m_activeTabIndex >= 0 && m_activeTabIndex < (int)m_tabs.size()) {
        const auto& tab = m_tabs[m_activeTabIndex];
        std::wstring windowTitle = tab.title + L" - RinganBrowser";
        SetWindowTextW(m_hWnd, windowTitle.c_str());

        // Don't show startpage file:/// path in edit box, show clean placeholder or "ringan://start"
        std::wstring displayUrl = tab.url;
        if (displayUrl.find(L"startpage.html") != std::wstring::npos) {
            displayUrl = L"ringan://start";
        }
        SetWindowTextW(m_hUrlEdit, displayUrl.c_str());
    }
}

void BrowserApp::ResizeViews() {
    RECT rcClient;
    GetClientRect(m_hWnd, &rcClient);
    m_clientW = rcClient.right - rcClient.left;
    m_clientH = rcClient.bottom - rcClient.top;

    if (m_clientW <= 0 || m_clientH <= 0) return;

    // Resize URL Edit control inside URL bar rect
    RECT rcUrl = GetUrlBarRect();
    int editX = rcUrl.left + 28; // Space for lock icon
    int editY = rcUrl.top + 6;
    int editW = (rcUrl.right - rcUrl.left) - 34;
    int editH = 22;

    MoveWindow(m_hUrlEdit, editX, editY, editW, editH, TRUE);

    // Resize active WebView controller
    RECT bounds;
    bounds.left = 0;
    bounds.top = Config::TOP_BAR_HEIGHT;
    bounds.right = m_clientW;
    bounds.bottom = m_clientH;

    if (m_activeTabIndex >= 0 && m_activeTabIndex < (int)m_tabs.size()) {
        auto& tab = m_tabs[m_activeTabIndex];
        if (tab.controller) {
            tab.controller->put_Bounds(bounds);
        }
    }
}

RECT BrowserApp::GetTabRect(int index) const {
    int x = 8 + index * (Config::TAB_MAX_WIDTH + 4);
    int y = 4;
    int w = Config::TAB_MAX_WIDTH;
    int h = Config::TAB_BAR_HEIGHT - 4;
    return RECT{ x, y, x + w, y + h };
}

RECT BrowserApp::GetTabCloseRect(int index) const {
    RECT tabRc = GetTabRect(index);
    int s = 16;
    int x = tabRc.right - 22;
    int y = tabRc.top + (tabRc.bottom - tabRc.top - s) / 2;
    return RECT{ x, y, x + s, y + s };
}

RECT BrowserApp::GetNewTabBtnRect() const {
    int x = 8 + (int)m_tabs.size() * (Config::TAB_MAX_WIDTH + 4);
    int y = 6;
    int s = Config::TAB_NEW_BTN_WIDTH - 6;
    return RECT{ x, y, x + s, y + s };
}

RECT BrowserApp::GetNavBtnRect(int btnIndex) const {
    // btnIndex: 0: Back, 1: Forward, 2: Reload, 3: Home, 4: Bookmark, 5: Download, 6: Zoom, 7: DevTools, 8: Settings
    int y = Config::TAB_BAR_HEIGHT + (Config::NAV_BAR_HEIGHT - Config::NAV_BTN_SIZE) / 2;
    int s = Config::NAV_BTN_SIZE;

    if (btnIndex < 4) {
        int x = 10 + btnIndex * (s + 4);
        return RECT{ x, y, x + s, y + s };
    } else {
        // Right side buttons (5 buttons: 4..8)
        int offsetFromRight = (9 - btnIndex);
        int x = m_clientW - 10 - offsetFromRight * (s + 4);
        return RECT{ x, y, x + s, y + s };
    }
}

RECT BrowserApp::GetUrlBarRect() const {
    RECT rcHome = GetNavBtnRect(3);
    RECT rcBookmark = GetNavBtnRect(4);
    int x = rcHome.right + 12;
    int y = Config::TAB_BAR_HEIGHT + 5;
    int r = rcBookmark.left - 12;
    int h = Config::NAV_BAR_HEIGHT - 10;
    return RECT{ x, y, r, y + h };
}

void BrowserApp::CheckHover(int x, int y) {
    HoverArea oldArea = m_hoverArea;
    int oldIndex = m_hoverIndex;

    m_hoverArea = HoverArea::None;
    m_hoverIndex = -1;

    // Check tabs
    for (int i = 0; i < (int)m_tabs.size(); ++i) {
        RECT closeRc = GetTabCloseRect(i);
        if (x >= closeRc.left && x <= closeRc.right && y >= closeRc.top && y <= closeRc.bottom) {
            m_hoverArea = HoverArea::TabClose;
            m_hoverIndex = i;
            break;
        }
        RECT tabRc = GetTabRect(i);
        if (x >= tabRc.left && x <= tabRc.right && y >= tabRc.top && y <= tabRc.bottom) {
            m_hoverArea = HoverArea::TabItem;
            m_hoverIndex = i;
            break;
        }
    }

    if (m_hoverArea == HoverArea::None) {
        RECT newTabRc = GetNewTabBtnRect();
        if (x >= newTabRc.left && x <= newTabRc.right && y >= newTabRc.top && y <= newTabRc.bottom) {
            m_hoverArea = HoverArea::BtnNewTab;
        }
    }

    // Check nav buttons
    if (m_hoverArea == HoverArea::None) {
        for (int i = 0; i < 9; ++i) {
            RECT btnRc = GetNavBtnRect(i);
            if (x >= btnRc.left && x <= btnRc.right && y >= btnRc.top && y <= btnRc.bottom) {
                switch (i) {
                    case 0: m_hoverArea = HoverArea::BtnBack; break;
                    case 1: m_hoverArea = HoverArea::BtnForward; break;
                    case 2: m_hoverArea = HoverArea::BtnReload; break;
                    case 3: m_hoverArea = HoverArea::BtnHome; break;
                    case 4: m_hoverArea = HoverArea::BtnBookmark; break;
                    case 5: m_hoverArea = HoverArea::BtnDownload; break;
                    case 6: m_hoverArea = HoverArea::BtnZoom; break;
                    case 7: m_hoverArea = HoverArea::BtnDevTools; break;
                    case 8: m_hoverArea = HoverArea::BtnSettings; break;
                }
                break;
            }
        }
    }

    if (oldArea != m_hoverArea || oldIndex != m_hoverIndex) {
        RECT rcTop = { 0, 0, m_clientW, Config::TOP_BAR_HEIGHT };
        InvalidateRect(m_hWnd, &rcTop, FALSE);
    }
}

void BrowserApp::HandleClick(int x, int y) {
    // Tabs close
    for (int i = 0; i < (int)m_tabs.size(); ++i) {
        RECT closeRc = GetTabCloseRect(i);
        if (x >= closeRc.left && x <= closeRc.right && y >= closeRc.top && y <= closeRc.bottom) {
            CloseTab(i);
            return;
        }
        RECT tabRc = GetTabRect(i);
        if (x >= tabRc.left && x <= tabRc.right && y >= tabRc.top && y <= tabRc.bottom) {
            SwitchTab(i);
            return;
        }
    }

    // New Tab
    RECT newTabRc = GetNewTabBtnRect();
    if (x >= newTabRc.left && x <= newTabRc.right && y >= newTabRc.top && y <= newTabRc.bottom) {
        NewTab();
        return;
    }

    // Nav buttons
    for (int i = 0; i < 9; ++i) {
        RECT btnRc = GetNavBtnRect(i);
        if (x >= btnRc.left && x <= btnRc.right && y >= btnRc.top && y <= btnRc.bottom) {
            switch (i) {
                case 0: GoBack(); break;
                case 1: GoForward(); break;
                case 2: Reload(); break;
                case 3: GoHome(); break;
                case 4: {
                    m_isBookmarked = !m_isBookmarked;
                    InvalidateRect(m_hWnd, nullptr, FALSE);
                    break;
                }
                case 5: OpenDownloadsFolder(); break;
                case 6: ZoomReset(); break;
                case 7: ToggleDevTools(); break;
                case 8: ShowSettingsMenu(); break;
            }
            return;
        }
    }
}

void BrowserApp::PaintChrome(HDC hdc) {
    if (m_clientW <= 0) return;

    // Double-buffered drawing
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, m_clientW, Config::TOP_BAR_HEIGHT);
    HGDIOBJ oldBmp = SelectObject(memDC, memBmp);

    Graphics g(memDC);
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    // 1. Paint Title/Tab bar background
    SolidBrush bgTabBrush(Color(255, 15, 23, 42)); // Slate 900
    g.FillRectangle(&bgTabBrush, 0, 0, m_clientW, Config::TAB_BAR_HEIGHT);

    // 2. Paint Navigation bar background
    SolidBrush bgNavBrush(Color(255, 30, 41, 59)); // Slate 800
    g.FillRectangle(&bgNavBrush, 0, Config::TAB_BAR_HEIGHT, m_clientW, Config::NAV_BAR_HEIGHT);

    // Bottom border line of nav bar
    Pen borderPen(Color(255, 51, 65, 85), 1.0f);
    g.DrawLine(&borderPen, 0, Config::TOP_BAR_HEIGHT - 1, m_clientW, Config::TOP_BAR_HEIGHT - 1);

    // 3. Paint Tabs
    FontFamily fontFamily(L"Segoe UI");
    Font fontTab(&fontFamily, 9.5f, FontStyleRegular, UnitPoint);
    StringFormat strFormat;
    strFormat.SetAlignment(StringAlignmentNear);
    strFormat.SetLineAlignment(StringAlignmentCenter);
    strFormat.SetTrimming(StringTrimmingEllipsisCharacter);
    strFormat.SetFormatFlags(StringFormatFlagsNoWrap);

    for (int i = 0; i < (int)m_tabs.size(); ++i) {
        const auto& tab = m_tabs[i];
        RECT rc = GetTabRect(i);
        bool isActive = (i == m_activeTabIndex);
        bool isHover = (m_hoverArea == HoverArea::TabItem && m_hoverIndex == i);

        Color tabBgColor = isActive ? Color(255, 30, 41, 59) :
                           isHover ? Color(255, 51, 65, 85) : Color(255, 20, 29, 48);

        SolidBrush tabBrush(tabBgColor);
        RectF tabRectF((float)rc.left, (float)rc.top, (float)(rc.right - rc.left), (float)(rc.bottom - rc.top));

        // Draw rounded tab background
        GraphicsPath tabPath;
        float r = 6.0f;
        tabPath.AddLine(tabRectF.X, tabRectF.GetBottom(), tabRectF.X, tabRectF.Y + r);
        tabPath.AddArc(tabRectF.X, tabRectF.Y, r * 2, r * 2, 180, 90);
        tabPath.AddLine(tabRectF.X + r, tabRectF.Y, tabRectF.GetRight() - r, tabRectF.Y);
        tabPath.AddArc(tabRectF.GetRight() - r * 2, tabRectF.Y, r * 2, r * 2, 270, 90);
        tabPath.AddLine(tabRectF.GetRight(), tabRectF.Y + r, tabRectF.GetRight(), tabRectF.GetBottom());
        g.FillPath(&tabBrush, &tabPath);

        // Active tab top indicator accent line
        if (isActive) {
            Pen accentPen(Color(255, 99, 102, 241), 2.5f);
            g.DrawLine(&accentPen, tabRectF.X + 2, tabRectF.Y + 1, tabRectF.GetRight() - 2, tabRectF.Y + 1);
        }

        // Draw Globe / Loading Icon
        float iconCx = tabRectF.X + 16.0f;
        float iconCy = tabRectF.Y + tabRectF.Height * 0.5f;
        if (tab.isLoading) {
            VectorIcons::DrawReloadIcon(g, iconCx, iconCy, 16.0f, Color(255, 99, 102, 241), false);
        } else {
            VectorIcons::DrawGlobeIcon(g, iconCx, iconCy, 16.0f, isActive ? Color(255, 99, 102, 241) : Color(255, 148, 163, 184));
        }

        // Tab Title Text
        RectF textRectF(tabRectF.X + 30.0f, tabRectF.Y, tabRectF.Width - 56.0f, tabRectF.Height);
        SolidBrush textBrush(isActive ? Color(255, 248, 250, 252) : Color(255, 148, 163, 184));
        g.DrawString(tab.title.c_str(), -1, &fontTab, textRectF, &strFormat, &textBrush);

        // Tab Close Button
        RECT closeRc = GetTabCloseRect(i);
        bool isCloseHover = (m_hoverArea == HoverArea::TabClose && m_hoverIndex == i);
        if (isCloseHover) {
            SolidBrush closeHoverBrush(Color(255, 239, 68, 68)); // Red hover
            g.FillEllipse(&closeHoverBrush, (float)closeRc.left, (float)closeRc.top, 16.0f, 16.0f);
        }
        VectorIcons::DrawCloseIcon(g, closeRc.left + 8.0f, closeRc.top + 8.0f, 16.0f,
            isCloseHover ? Color(255, 255, 255, 255) : Color(255, 148, 163, 184));
    }

    // 4. Paint New Tab '+' button
    RECT newTabRc = GetNewTabBtnRect();
    bool isNewTabHover = (m_hoverArea == HoverArea::BtnNewTab);
    if (isNewTabHover) {
        SolidBrush hoverBrush(Color(255, 51, 65, 85));
        g.FillEllipse(&hoverBrush, (float)newTabRc.left, (float)newTabRc.top, (float)(newTabRc.right - newTabRc.left), (float)(newTabRc.bottom - newTabRc.top));
    }
    VectorIcons::DrawPlusIcon(g, newTabRc.left + (newTabRc.right - newTabRc.left) * 0.5f,
        newTabRc.top + (newTabRc.bottom - newTabRc.top) * 0.5f, 18.0f,
        isNewTabHover ? Color(255, 255, 255, 255) : Color(255, 148, 163, 184));

    // 5. Paint Navigation Buttons
    auto drawNavButton = [&](int btnIdx, HoverArea hoverVal, auto drawIconFn) {
        RECT rc = GetNavBtnRect(btnIdx);
        bool isHov = (m_hoverArea == hoverVal);
        if (isHov) {
            SolidBrush hBrush(Color(255, 51, 65, 85));
            g.FillEllipse(&hBrush, (float)rc.left, (float)rc.top, (float)(rc.right - rc.left), (float)(rc.bottom - rc.top));
        }
        float cx = rc.left + (rc.right - rc.left) * 0.5f;
        float cy = rc.top + (rc.bottom - rc.top) * 0.5f;
        Color iconCol = isHov ? Color(255, 255, 255, 255) : Color(255, 203, 213, 225);
        drawIconFn(cx, cy, iconCol);
    };

    // Back
    drawNavButton(0, HoverArea::BtnBack, [&](float cx, float cy, Color col) {
        VectorIcons::DrawBackIcon(g, cx, cy, 22.0f, col, true);
    });

    // Forward
    drawNavButton(1, HoverArea::BtnForward, [&](float cx, float cy, Color col) {
        VectorIcons::DrawForwardIcon(g, cx, cy, 22.0f, col, true);
    });

    // Reload
    drawNavButton(2, HoverArea::BtnReload, [&](float cx, float cy, Color col) {
        bool isLoading = (m_activeTabIndex >= 0 && m_activeTabIndex < (int)m_tabs.size() && m_tabs[m_activeTabIndex].isLoading);
        VectorIcons::DrawReloadIcon(g, cx, cy, 22.0f, col, isLoading);
    });

    // Home
    drawNavButton(3, HoverArea::BtnHome, [&](float cx, float cy, Color col) {
        VectorIcons::DrawHomeIcon(g, cx, cy, 22.0f, col);
    });

    // 6. Paint URL Bar Rounded Box
    RECT rcUrl = GetUrlBarRect();
    RectF urlRectF((float)rcUrl.left, (float)rcUrl.top, (float)(rcUrl.right - rcUrl.left), (float)(rcUrl.bottom - rcUrl.top));
    
    // Rounded URL background
    SolidBrush urlBgBrush(Color(255, 15, 23, 42)); // Slate 900
    Pen urlBorderPen(Color(255, 71, 85, 105), 1.0f); // Slate 600
    
    GraphicsPath urlPath;
    float ur = urlRectF.Height * 0.5f;
    urlPath.AddArc(urlRectF.X, urlRectF.Y, ur * 2, ur * 2, 90, 180);
    urlPath.AddArc(urlRectF.GetRight() - ur * 2, urlRectF.Y, ur * 2, ur * 2, 270, 180);
    urlPath.CloseFigure();
    
    g.FillPath(&urlBgBrush, &urlPath);
    g.DrawPath(&urlBorderPen, &urlPath);

    // Draw SSL Lock Icon in URL Bar
    bool isSecure = (m_activeTabIndex >= 0 && m_activeTabIndex < (int)m_tabs.size() && m_tabs[m_activeTabIndex].isSecure);
    VectorIcons::DrawLockIcon(g, urlRectF.X + 16.0f, urlRectF.Y + urlRectF.Height * 0.5f, 16.0f, isSecure);

    // 7. Paint Right-side Buttons
    // Bookmark
    drawNavButton(4, HoverArea::BtnBookmark, [&](float cx, float cy, Color col) {
        VectorIcons::DrawStarIcon(g, cx, cy, 20.0f, m_isBookmarked);
    });

    // Download
    drawNavButton(5, HoverArea::BtnDownload, [&](float cx, float cy, Color col) {
        VectorIcons::DrawDownloadIcon(g, cx, cy, 20.0f, col);
    });

    // Zoom Indicator
    drawNavButton(6, HoverArea::BtnZoom, [&](float cx, float cy, Color col) {
        double z = (m_activeTabIndex >= 0 && m_activeTabIndex < (int)m_tabs.size()) ? m_tabs[m_activeTabIndex].zoomFactor : 1.0;
        int zoomPercent = (int)(z * 100.0 + 0.5);
        std::wstring zStr = std::to_wstring(zoomPercent) + L"%";
        Font fontZoom(&fontFamily, 8.0f, FontStyleBold, UnitPoint);
        StringFormat sfCenter;
        sfCenter.SetAlignment(StringAlignmentCenter);
        sfCenter.SetLineAlignment(StringAlignmentCenter);
        SolidBrush zBrush(col);
        g.DrawString(zStr.c_str(), -1, &fontZoom, PointF(cx, cy), &sfCenter, &zBrush);
    });

    // DevTools
    drawNavButton(7, HoverArea::BtnDevTools, [&](float cx, float cy, Color col) {
        VectorIcons::DrawDevToolsIcon(g, cx, cy, 22.0f, col);
    });

    // Settings / Info
    drawNavButton(8, HoverArea::BtnSettings, [&](float cx, float cy, Color col) {
        VectorIcons::DrawSettingsIcon(g, cx, cy, 22.0f, col);
    });

    // BitBlt to screen
    BitBlt(hdc, 0, 0, m_clientW, Config::TOP_BAR_HEIGHT, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);
}

LRESULT BrowserApp::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            PaintChrome(hdc);
            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1; // Prevent flicker

        case WM_SIZE: {
            ResizeViews();
            InvalidateRect(hWnd, nullptr, FALSE);
            return 0;
        }

        case WM_MOUSEMOVE: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            CheckHover(x, y);

            TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, hWnd, 0 };
            TrackMouseEvent(&tme);
            return 0;
        }

        case WM_MOUSELEAVE: {
            m_hoverArea = HoverArea::None;
            m_hoverIndex = -1;
            RECT rcTop = { 0, 0, m_clientW, Config::TOP_BAR_HEIGHT };
            InvalidateRect(hWnd, &rcTop, FALSE);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            if (y < Config::TOP_BAR_HEIGHT) {
                HandleClick(x, y);
            }
            return 0;
        }

        case WM_CTLCOLOREDIT: {
            HDC hdcEdit = (HDC)wParam;
            SetTextColor(hdcEdit, Config::COLOR_TEXT_MAIN);
            SetBkColor(hdcEdit, Config::COLOR_URL_BG);
            static HBRUSH hBrushUrl = CreateSolidBrush(Config::COLOR_URL_BG);
            return (LRESULT)hBrushUrl;
        }

        case WM_KEYDOWN: {
            bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            bool alt = (GetKeyState(VK_MENU) & 0x8000) != 0;

            if (wParam == VK_F5) {
                Reload();
                return 0;
            } else if (wParam == VK_F12) {
                ToggleDevTools();
                return 0;
            } else if (ctrl && wParam == 'T') {
                NewTab();
                return 0;
            } else if (ctrl && wParam == 'W') {
                CloseTab(m_activeTabIndex);
                return 0;
            } else if (ctrl && wParam == 'R') {
                Reload();
                return 0;
            } else if (ctrl && wParam == 'J') {
                OpenDownloadsFolder();
                return 0;
            } else if (ctrl && wParam == 'L' || (alt && wParam == 'D')) {
                SetFocus(m_hUrlEdit);
                SendMessageW(m_hUrlEdit, EM_SETSEL, 0, -1);
                return 0;
            } else if (ctrl && (wParam == VK_OEM_PLUS || wParam == VK_ADD)) {
                ZoomIn();
                return 0;
            } else if (ctrl && (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT)) {
                ZoomOut();
                return 0;
            } else if (ctrl && (wParam == '0' || wParam == VK_NUMPAD0)) {
                ZoomReset();
                return 0;
            } else if (ctrl && wParam == 'H') {
                GoHome();
                return 0;
            } else if (alt && wParam == VK_LEFT) {
                GoBack();
                return 0;
            } else if (alt && wParam == VK_RIGHT) {
                GoForward();
                return 0;
            } else if (ctrl && wParam == VK_TAB) {
                if (!m_tabs.empty()) {
                    int next = (m_activeTabIndex + 1) % (int)m_tabs.size();
                    SwitchTab(next);
                }
                return 0;
            }
            break;
        }

        case WM_CLOSE: {
            DestroyWindow(hWnd);
            return 0;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
