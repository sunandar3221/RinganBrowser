package com.ringan.browser;

import android.app.AlertDialog;
import android.app.DownloadManager;
import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.webkit.CookieManager;
import android.webkit.DownloadListener;
import android.webkit.JavascriptInterface;
import android.webkit.URLUtil;
import android.webkit.WebChromeClient;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.swiperefreshlayout.widget.SwipeRefreshLayout;

public class MainActivity extends AppCompatActivity {

    private static final String PREFS_NAME = "ringan_prefs";
    private static final String PREF_SEARCH_ENGINE = "search_engine";
    private static final String START_PAGE_URL = "file:///android_asset/startpage.html";

    private WebView webView;
    private EditText editUrl;
    private ProgressBar progressBar;
    private SwipeRefreshLayout swipeRefresh;
    private ImageView iconSsl;
    private ImageButton btnBack;
    private ImageButton btnForward;
    private SharedPreferences prefs;
    private String currentSearchEngine = "google";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        prefs = getSharedPreferences(PREFS_NAME, MODE_PRIVATE);
        currentSearchEngine = prefs.getString(PREF_SEARCH_ENGINE, "google");

        initViews();
        setupWebView();
        setupListeners();

        // Load startpage
        webView.loadUrl(START_PAGE_URL);
    }

    private void initViews() {
        webView = findViewById(R.id.web_view);
        editUrl = findViewById(R.id.edit_url);
        progressBar = findViewById(R.id.progress_bar);
        swipeRefresh = findViewById(R.id.swipe_refresh);
        iconSsl = findViewById(R.id.icon_ssl);
        btnBack = findViewById(R.id.btn_back);
        btnForward = findViewById(R.id.btn_forward);
    }

    private void setupWebView() {
        WebSettings settings = webView.getSettings();
        settings.setJavaScriptEnabled(true);
        settings.setDomStorageEnabled(true);
        settings.setDatabaseEnabled(true);
        settings.setAllowFileAccess(true);
        settings.setAllowContentAccess(true);
        settings.setBuiltInZoomControls(true);
        settings.setDisplayZoomControls(false);
        settings.setLoadWithOverviewMode(true);
        settings.setUseWideViewPort(true);
        settings.setMixedContentMode(WebSettings.MIXED_CONTENT_COMPATIBILITY_MODE);

        webView.addJavascriptInterface(new Object() {
            @JavascriptInterface
            public String getSearchEngine() {
                return currentSearchEngine;
            }
            @JavascriptInterface
            public void setSearchEngine(String engine) {
                runOnUiThread(() -> setSearchEngineInternal(engine));
            }
            @JavascriptInterface
            public void openSettings() {
                runOnUiThread(() -> showSettingsDialog());
            }
        }, "RinganNative");

        webView.setWebChromeClient(new WebChromeClient() {
            @Override
            public void onProgressChanged(WebView view, int newProgress) {
                if (newProgress < 100) {
                    progressBar.setVisibility(View.VISIBLE);
                    progressBar.setProgress(newProgress);
                } else {
                    progressBar.setVisibility(View.GONE);
                    swipeRefresh.setRefreshing(false);
                }
            }
        });

        webView.setWebViewClient(new WebViewClient() {
            @Override
            public void onPageStarted(WebView view, String url, Bitmap favicon) {
                updateUrlDisplay(url);
            }

            @Override
            public void onPageFinished(WebView view, String url) {
                updateUrlDisplay(url);
                updateNavButtons();
                if (url != null && url.contains("startpage.html")) {
                    syncSearchEngineToStartPage();
                }
            }
        });

        // Download handling
        webView.setDownloadListener(new DownloadListener() {
            @Override
            public void onDownloadStart(String url, String userAgent, String contentDisposition, String mimeType, long contentLength) {
                try {
                    DownloadManager.Request request = new DownloadManager.Request(Uri.parse(url));
                    request.setMimeType(mimeType);
                    String cookies = CookieManager.getInstance().getCookie(url);
                    request.addRequestHeader("cookie", cookies);
                    request.addRequestHeader("User-Agent", userAgent);
                    request.setDescription(getString(R.string.download_started));
                    String filename = URLUtil.guessFileName(url, contentDisposition, mimeType);
                    request.setTitle(filename);
                    request.allowScanningByMediaScanner();
                    request.setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED);
                    request.setDestinationInExternalPublicDir(Environment.DIRECTORY_DOWNLOADS, filename);

                    DownloadManager dm = (DownloadManager) getSystemService(Context.DOWNLOAD_SERVICE);
                    if (dm != null) {
                        dm.enqueue(request);
                        Toast.makeText(MainActivity.this, "Mengunduh " + filename, Toast.LENGTH_SHORT).show();
                    }
                } catch (Exception e) {
                    Toast.makeText(MainActivity.this, "Gagal mengunduh: " + e.getMessage(), Toast.LENGTH_SHORT).show();
                }
            }
        });
    }

    private void setupListeners() {
        // Omnibar Enter / Go
        editUrl.setOnEditorActionListener((v, actionId, event) -> {
            if (actionId == EditorInfo.IME_ACTION_GO ||
                (event != null && event.getKeyCode() == KeyEvent.KEYCODE_ENTER && event.getAction() == KeyEvent.ACTION_DOWN)) {
                navigateTo(editUrl.getText().toString().trim());
                hideKeyboard();
                return true;
            }
            return false;
        });

        // Swipe Refresh
        swipeRefresh.setOnRefreshListener(() -> webView.reload());

        // Buttons
        btnBack.setOnClickListener(v -> {
            if (webView.canGoBack()) webView.goBack();
        });

        btnForward.setOnClickListener(v -> {
            if (webView.canGoForward()) webView.goForward();
        });

        findViewById(R.id.btn_home).setOnClickListener(v -> webView.loadUrl(START_PAGE_URL));

        findViewById(R.id.btn_refresh).setOnClickListener(v -> webView.reload());

        View btnSettings = findViewById(R.id.btn_settings);
        if (btnSettings != null) {
            btnSettings.setOnClickListener(v -> showSettingsDialog());
        }
    }

    private String getSearchEngineBaseUrl() {
        if ("bing".equals(currentSearchEngine)) {
            return "https://www.bing.com/search?q=";
        } else if ("duckduckgo".equals(currentSearchEngine)) {
            return "https://duckduckgo.com/html/?q=";
        } else if ("wikipedia".equals(currentSearchEngine)) {
            return "https://id.wikipedia.org/wiki/Special:Search?search=";
        }
        return "https://www.google.com/search?q=";
    }

    private void setSearchEngineInternal(String engine) {
        currentSearchEngine = engine;
        prefs.edit().putString(PREF_SEARCH_ENGINE, engine).apply();
        syncSearchEngineToStartPage();
    }

    private void syncSearchEngineToStartPage() {
        String js = "if (typeof updateSearchEngineBadge === 'function') { updateSearchEngineBadge('" + currentSearchEngine + "'); }";
        webView.evaluateJavascript(js, null);
    }

    private void showSettingsDialog() {
        final String[] engines = {"Google", "Bing", "DuckDuckGo", "Wikipedia"};
        final String[] engineKeys = {"google", "bing", "duckduckgo", "wikipedia"};

        int checkedItem = 0;
        for (int i = 0; i < engineKeys.length; i++) {
            if (engineKeys[i].equals(currentSearchEngine)) {
                checkedItem = i;
                break;
            }
        }

        new AlertDialog.Builder(this)
            .setTitle(R.string.choose_search_engine)
            .setSingleChoiceItems(engines, checkedItem, (dialog, which) -> {
                setSearchEngineInternal(engineKeys[which]);
                Toast.makeText(this, "Mesin pencari diubah ke " + engines[which], Toast.LENGTH_SHORT).show();
                dialog.dismiss();
            })
            .setNegativeButton("Tutup", null)
            .show();
    }

    private void navigateTo(String input) {
        if (input.isEmpty()) {
            webView.loadUrl(START_PAGE_URL);
            return;
        }

        String finalUrl;
        if (input.startsWith("http://") || input.startsWith("https://") || input.startsWith("file:///")) {
            finalUrl = input;
        } else if (input.contains(".") && !input.contains(" ")) {
            finalUrl = "https://" + input;
        } else {
            finalUrl = getSearchEngineBaseUrl() + Uri.encode(input);
        }

        webView.loadUrl(finalUrl);
    }

    private void updateUrlDisplay(String url) {
        if (url == null) return;
        if (url.contains("startpage.html")) {
            editUrl.setText("");
            iconSsl.setColorFilter(0xFF38BDF8); // Cyan
        } else {
            editUrl.setText(url);
            if (url.startsWith("https://")) {
                iconSsl.setColorFilter(0xFF4ADE80); // Green
            } else {
                iconSsl.setColorFilter(0xFF94A3B8); // Slate
            }
        }
    }

    private void updateNavButtons() {
        btnBack.setEnabled(webView.canGoBack());
        btnBack.setAlpha(webView.canGoBack() ? 1.0f : 0.4f);
        btnForward.setEnabled(webView.canGoForward());
        btnForward.setAlpha(webView.canGoForward() ? 1.0f : 0.4f);
    }

    private void hideKeyboard() {
        View view = getCurrentFocus();
        if (view != null) {
            InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
            if (imm != null) imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
        }
        webView.requestFocus();
    }

    @Override
    public void onBackPressed() {
        if (webView.canGoBack()) {
            webView.goBack();
        } else {
            super.onBackPressed();
        }
    }
}
