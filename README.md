# RinganBrowser 🚀🍃

**RinganBrowser** adalah peramban web modern, ultra ringan, dan responsif yang ditulis murni dalam **C++20** (Win32 API) untuk sistem operasi Windows.

Browser ini dirancang khusus **tanpa mem-fork Chromium dari source code**, sehingga:
- ❌ **TIDAK memerlukan spec PC tinggi** (tidak perlu RAM 32GB atau storage 100GB untuk clone & kompilasi Chromium).
- ⚡ **Build super cepat**: Hanya butuh waktu **~3-5 detik** menggunakan GCC / MinGW (`w64devkit`).
- 📦 **Ukuran binary sangat kecil**: Executable hanya berukuran **~800 KB**!
- 🎨 **UI Modern & Ikon Vektor / SVG**: Desain Glassmorphism dark mode yang elegan dengan tombol navigasi bervektor tajam dan start page dengan grafis SVG interaktif.
- 🌐 **Dukungan Standar Web Lengkap**: Mendukung HTML5 (Canvas, Video, Audio), CSS3 (Flexbox, Grid, Glassmorphism, Animasi), JavaScript (ES2024, Async/Await), SVG murni, WebGL, dan DevTools (F12) melalui Evergreen WebView2 runtime bawaan Windows.

---

## ✨ Fitur Utama

1. **Native C++ Performance**:
   - Dibangun di atas Win32 API native dan GDI+ untuk rendering grafis vektor antialiased tanpa lag.
   - Konsumsi RAM background sangat minimal.
2. **Multi-Tab Browsing**:
   - Mendukung banyak tab sekaligus dengan tombol `+` (Tab Baru) dan `×` (Tutup Tab).
   - Setiap tab memiliki judul dinamis, status loading, dan histori navigasi sendiri.
3. **Omnibar Cerdas (URL & Search)**:
   - Mendeteksi otomatis alamat URL langsung (`https://...`, `google.com`, `wikipedia.org`, file lokal `file:///...`) maupun kueri pencarian (DuckDuckGo, Google).
   - Indikator keamanan SSL dengan lencana gembok (hijau untuk HTTPS aman).
4. **Pengelola Unduhan (Download Manager)**:
   - Akses cepat folder unduhan lewat tombol toolbar `⬇` atau shortcut `Ctrl + J`.
   - Terintegrasi langsung dengan direktori unduhan pengguna.
5. **Halaman Mulai (Start Page) Interaktif**:
   - Logo SVG animasi futuristik RinganBrowser.
   - Pilihan mesin pencari (DuckDuckGo, Google, Bing, Wikipedia).
   - Speed Dial dengan ikon logo brand asli (Google, YouTube, GitHub, Wikipedia, Reddit, MDN, Discord, DuckDuckGo).
   - Live Showcase interaktif: simulasi partikel HTML5 Canvas 60 FPS, tes animasi transform SVG & CSS3, sintesis audio Web Audio API, serta monitor status diagnostik browser.
6. **Developer Tools Bawaan**:
   - Akses penuh ke Inspect Element, Console, Network, dan Source via shortcut **F12** atau tombol Developer Tools pada toolbar.
7. **Kontrol Zoom & Bookmark**:
   - Tombol bintang bookmark dan indikator zoom persentase (reset ke 100%).
8. **Dukungan Multi-Platform**:
   - **Windows**: Tersedia varian **Setup Installer** (`.exe`) dan **Portable** (`.zip`).
   - **Android**: Proyek Android native ultra-ringan berbasis WebView yang otomatis dibangun via GitHub Actions CI/CD.

---

## ⌨️ Pintasan Keyboard (Shortcuts)

| Pintasan | Fungsi |
| :--- | :--- |
| `Ctrl + T` | Buka Tab Baru |
| `Ctrl + W` | Tutup Tab Aktif |
| `Ctrl + L` / `Alt + D` | Fokus ke Bar Alamat (URL Bar) |
| `Ctrl + R` / `F5` | Muat Ulang Halaman (Reload) |
| `Ctrl + J` | Buka Pengelola Unduhan (Downloads) |
| `Alt + Panah Kiri` | Mundur ke halaman sebelumnya (Back) |
| `Alt + Panah Kanan` | Maju ke halaman berikutnya (Forward) |
| `Ctrl + H` | Buka Halaman Awal (Home / Start Page) |
| `Ctrl + Tab` | Pindah ke Tab Berikutnya |
| `Ctrl + +` / `Ctrl + -` | Perbesar / Perkecil Halaman (Zoom In / Out) |
| `Ctrl + 0` | Reset Zoom ke 100% |
| `F12` | Buka Developer Tools (Inspect Element) |

---

## 🛠️ Cara Build

RinganBrowser dapat dikompilasi menggunakan compiler GCC di w64devkit atau MinGW-w64:

1. Buka PowerShell atau Command Prompt di folder `RinganBrowser`.
2. Jalankan `build.bat`:
   ```cmd
   .\build.bat
   ```
3. Executable `RinganBrowser.exe` akan terbuat dalam hitungan detik.

---

## 📁 Struktur Proyek

```
RinganBrowser/
├── src/
│   ├── Config.h            # Konfigurasi dimensi, warna, & URL
│   ├── VectorIcons.h       # Fungsi render ikon vektor GDI+ (Back, Forward, Reload, dll)
│   ├── WebViewHandlers.h   # COM Event handlers untuk WebView2
│   ├── BrowserApp.h        # Deklarasi controller browser & manajemen tab
│   ├── BrowserApp.cpp      # Implementasi UI Win32 & logika peramban
│   └── main.cpp            # Titik masuk WinMain & inisialisasi GDI+/COM
├── assets/
│   ├── app.manifest        # Manifest DPI Awareness & visual styles
│   ├── icon.ico            # Ikon resolusi tinggi aplikasi
│   └── startpage.html      # Start page interaktif dengan SVG, HTML5 Canvas & JS
├── resource.rc             # Win32 resource file
├── WebView2Loader.dll      # DLL loader ringan (~160 KB)
├── build.bat               # Skrip kompilasi otomatis
├── installer.iss           # Skrip installer Inno Setup Windows
├── android/                # Proyek Android WebView (Gradle)
└── README.md               # Dokumentasi proyek
```

---

## 📄 Lisensi

Proyek ini dilisensikan di bawah lisensi **MIT License** - lihat file [LICENSE](LICENSE) untuk rincian lengkap.

