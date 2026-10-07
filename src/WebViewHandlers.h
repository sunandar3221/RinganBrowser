#pragma once
#include <windows.h>
#include <functional>
#include "WebView2.h"

namespace WebViewHandlers {

    class EnvironmentHandler : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler {
        LONG m_ref = 1;
        std::function<HRESULT(HRESULT, ICoreWebView2Environment*)> m_cb;
    public:
        EnvironmentHandler(std::function<HRESULT(HRESULT, ICoreWebView2Environment*)> cb) : m_cb(cb) {}
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
            if (!ppv) return E_POINTER;
            if (InlineIsEqualGUID(riid, IID_IUnknown) || InlineIsEqualGUID(riid, IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler)) {
                *ppv = static_cast<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*>(this);
                AddRef();
                return S_OK;
            }
            *ppv = nullptr;
            return E_NOINTERFACE;
        }
        ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_ref); }
        ULONG STDMETHODCALLTYPE Release() override {
            LONG r = InterlockedDecrement(&m_ref);
            if (r == 0) delete this;
            return r;
        }
        HRESULT STDMETHODCALLTYPE Invoke(HRESULT res, ICoreWebView2Environment* env) override {
            return m_cb ? m_cb(res, env) : S_OK;
        }
    };

    class ControllerHandler : public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler {
        LONG m_ref = 1;
        std::function<HRESULT(HRESULT, ICoreWebView2Controller*)> m_cb;
    public:
        ControllerHandler(std::function<HRESULT(HRESULT, ICoreWebView2Controller*)> cb) : m_cb(cb) {}
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
            if (!ppv) return E_POINTER;
            if (InlineIsEqualGUID(riid, IID_IUnknown) || InlineIsEqualGUID(riid, IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler)) {
                *ppv = static_cast<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler*>(this);
                AddRef();
                return S_OK;
            }
            *ppv = nullptr;
            return E_NOINTERFACE;
        }
        ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_ref); }
        ULONG STDMETHODCALLTYPE Release() override {
            LONG r = InterlockedDecrement(&m_ref);
            if (r == 0) delete this;
            return r;
        }
        HRESULT STDMETHODCALLTYPE Invoke(HRESULT res, ICoreWebView2Controller* ctrl) override {
            return m_cb ? m_cb(res, ctrl) : S_OK;
        }
    };

    class NavigationStartingHandler : public ICoreWebView2NavigationStartingEventHandler {
        LONG m_ref = 1;
        std::function<HRESULT(ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs*)> m_cb;
    public:
        NavigationStartingHandler(std::function<HRESULT(ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs*)> cb) : m_cb(cb) {}
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
            if (!ppv) return E_POINTER;
            if (InlineIsEqualGUID(riid, IID_IUnknown) || InlineIsEqualGUID(riid, IID_ICoreWebView2NavigationStartingEventHandler)) {
                *ppv = static_cast<ICoreWebView2NavigationStartingEventHandler*>(this);
                AddRef();
                return S_OK;
            }
            *ppv = nullptr;
            return E_NOINTERFACE;
        }
        ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_ref); }
        ULONG STDMETHODCALLTYPE Release() override {
            LONG r = InterlockedDecrement(&m_ref);
            if (r == 0) delete this;
            return r;
        }
        HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args) override {
            return m_cb ? m_cb(sender, args) : S_OK;
        }
    };

    class NavigationCompletedHandler : public ICoreWebView2NavigationCompletedEventHandler {
        LONG m_ref = 1;
        std::function<HRESULT(ICoreWebView2*, ICoreWebView2NavigationCompletedEventArgs*)> m_cb;
    public:
        NavigationCompletedHandler(std::function<HRESULT(ICoreWebView2*, ICoreWebView2NavigationCompletedEventArgs*)> cb) : m_cb(cb) {}
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
            if (!ppv) return E_POINTER;
            if (InlineIsEqualGUID(riid, IID_IUnknown) || InlineIsEqualGUID(riid, IID_ICoreWebView2NavigationCompletedEventHandler)) {
                *ppv = static_cast<ICoreWebView2NavigationCompletedEventHandler*>(this);
                AddRef();
                return S_OK;
            }
            *ppv = nullptr;
            return E_NOINTERFACE;
        }
        ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_ref); }
        ULONG STDMETHODCALLTYPE Release() override {
            LONG r = InterlockedDecrement(&m_ref);
            if (r == 0) delete this;
            return r;
        }
        HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) override {
            return m_cb ? m_cb(sender, args) : S_OK;
        }
    };

    class SourceChangedHandler : public ICoreWebView2SourceChangedEventHandler {
        LONG m_ref = 1;
        std::function<HRESULT(ICoreWebView2*, ICoreWebView2SourceChangedEventArgs*)> m_cb;
    public:
        SourceChangedHandler(std::function<HRESULT(ICoreWebView2*, ICoreWebView2SourceChangedEventArgs*)> cb) : m_cb(cb) {}
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
            if (!ppv) return E_POINTER;
            if (InlineIsEqualGUID(riid, IID_IUnknown) || InlineIsEqualGUID(riid, IID_ICoreWebView2SourceChangedEventHandler)) {
                *ppv = static_cast<ICoreWebView2SourceChangedEventHandler*>(this);
                AddRef();
                return S_OK;
            }
            *ppv = nullptr;
            return E_NOINTERFACE;
        }
        ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_ref); }
        ULONG STDMETHODCALLTYPE Release() override {
            LONG r = InterlockedDecrement(&m_ref);
            if (r == 0) delete this;
            return r;
        }
        HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2* sender, ICoreWebView2SourceChangedEventArgs* args) override {
            return m_cb ? m_cb(sender, args) : S_OK;
        }
    };

    class DocumentTitleChangedHandler : public ICoreWebView2DocumentTitleChangedEventHandler {
        LONG m_ref = 1;
        std::function<HRESULT(ICoreWebView2*, IUnknown*)> m_cb;
    public:
        DocumentTitleChangedHandler(std::function<HRESULT(ICoreWebView2*, IUnknown*)> cb) : m_cb(cb) {}
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
            if (!ppv) return E_POINTER;
            if (InlineIsEqualGUID(riid, IID_IUnknown) || InlineIsEqualGUID(riid, IID_ICoreWebView2DocumentTitleChangedEventHandler)) {
                *ppv = static_cast<ICoreWebView2DocumentTitleChangedEventHandler*>(this);
                AddRef();
                return S_OK;
            }
            *ppv = nullptr;
            return E_NOINTERFACE;
        }
        ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_ref); }
        ULONG STDMETHODCALLTYPE Release() override {
            LONG r = InterlockedDecrement(&m_ref);
            if (r == 0) delete this;
            return r;
        }
        HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2* sender, IUnknown* args) override {
            return m_cb ? m_cb(sender, args) : S_OK;
        }
    };

    class HistoryChangedHandler : public ICoreWebView2HistoryChangedEventHandler {
        LONG m_ref = 1;
        std::function<HRESULT(ICoreWebView2*, IUnknown*)> m_cb;
    public:
        HistoryChangedHandler(std::function<HRESULT(ICoreWebView2*, IUnknown*)> cb) : m_cb(cb) {}
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
            if (!ppv) return E_POINTER;
            if (InlineIsEqualGUID(riid, IID_IUnknown) || InlineIsEqualGUID(riid, IID_ICoreWebView2HistoryChangedEventHandler)) {
                *ppv = static_cast<ICoreWebView2HistoryChangedEventHandler*>(this);
                AddRef();
                return S_OK;
            }
            *ppv = nullptr;
            return E_NOINTERFACE;
        }
        ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_ref); }
        ULONG STDMETHODCALLTYPE Release() override {
            LONG r = InterlockedDecrement(&m_ref);
            if (r == 0) delete this;
            return r;
        }
        HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2* sender, IUnknown* args) override {
            return m_cb ? m_cb(sender, args) : S_OK;
        }
    };

    class NewWindowRequestedHandler : public ICoreWebView2NewWindowRequestedEventHandler {
        LONG m_ref = 1;
        std::function<HRESULT(ICoreWebView2*, ICoreWebView2NewWindowRequestedEventArgs*)> m_cb;
    public:
        NewWindowRequestedHandler(std::function<HRESULT(ICoreWebView2*, ICoreWebView2NewWindowRequestedEventArgs*)> cb) : m_cb(cb) {}
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
            if (!ppv) return E_POINTER;
            if (InlineIsEqualGUID(riid, IID_IUnknown) || InlineIsEqualGUID(riid, IID_ICoreWebView2NewWindowRequestedEventHandler)) {
                *ppv = static_cast<ICoreWebView2NewWindowRequestedEventHandler*>(this);
                AddRef();
                return S_OK;
            }
            *ppv = nullptr;
            return E_NOINTERFACE;
        }
        ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_ref); }
        ULONG STDMETHODCALLTYPE Release() override {
            LONG r = InterlockedDecrement(&m_ref);
            if (r == 0) delete this;
            return r;
        }
        HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2* sender, ICoreWebView2NewWindowRequestedEventArgs* args) override {
            return m_cb ? m_cb(sender, args) : S_OK;
        }
    };

    class WebMessageReceivedHandler : public ICoreWebView2WebMessageReceivedEventHandler {
        LONG m_ref = 1;
        std::function<HRESULT(ICoreWebView2*, ICoreWebView2WebMessageReceivedEventArgs*)> m_cb;
    public:
        WebMessageReceivedHandler(std::function<HRESULT(ICoreWebView2*, ICoreWebView2WebMessageReceivedEventArgs*)> cb) : m_cb(cb) {}
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
            if (!ppv) return E_POINTER;
            if (InlineIsEqualGUID(riid, IID_IUnknown) || InlineIsEqualGUID(riid, IID_ICoreWebView2WebMessageReceivedEventHandler)) {
                *ppv = static_cast<ICoreWebView2WebMessageReceivedEventHandler*>(this);
                AddRef();
                return S_OK;
            }
            *ppv = nullptr;
            return E_NOINTERFACE;
        }
        ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_ref); }
        ULONG STDMETHODCALLTYPE Release() override {
            LONG r = InterlockedDecrement(&m_ref);
            if (r == 0) delete this;
            return r;
        }
        HRESULT STDMETHODCALLTYPE Invoke(ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) override {
            return m_cb ? m_cb(sender, args) : S_OK;
        }
    };

}
