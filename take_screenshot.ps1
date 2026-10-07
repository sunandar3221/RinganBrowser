Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

# Start RinganBrowser if not already running
$proc = Get-Process -Name "RinganBrowser" -ErrorAction SilentlyContinue
if (-not $proc) {
    Write-Host "Launching RinganBrowser.exe..."
    $proc = Start-Process -FilePath ".\RinganBrowser.exe" -PassThru
    Start-Sleep -Seconds 4
} else {
    Write-Host "RinganBrowser is already running with PID $($proc.Id)"
    Start-Sleep -Seconds 1
}

# Win32 definitions to find and bring window to front
$user32 = Add-Type -MemberDefinition @"
[DllImport("user32.dll")]
public static extern bool SetForegroundWindow(IntPtr hWnd);

[DllImport("user32.dll")]
public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

[DllImport("user32.dll")]
public static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);

[DllImport("user32.dll")]
public static extern bool PrintWindow(IntPtr hWnd, IntPtr hdcBmp, uint nFlags);

[StructLayout(LayoutKind.Sequential)]
public struct RECT {
    public int Left;
    public int Top;
    public int Right;
    public int Bottom;
}
"@ -Name "Win32Util" -Namespace "Win32" -PassThru

# Find MainWindowHandle
$proc.Refresh()
$hWnd = $proc.MainWindowHandle
$retry = 0
while ($hWnd -eq [IntPtr]::Zero -and $retry -lt 10) {
    Start-Sleep -Milliseconds 500
    $proc.Refresh()
    $hWnd = $proc.MainWindowHandle
    $retry++
}

Write-Host "Window Handle: $hWnd"

if ($hWnd -ne [IntPtr]::Zero) {
    [Win32.Win32Util]::ShowWindow($hWnd, 9) # SW_RESTORE
    [Win32.Win32Util]::SetForegroundWindow($hWnd)
    Start-Sleep -Seconds 2

    $rect = New-Object Win32.Win32Util+RECT
    [Win32.Win32Util]::GetWindowRect($hWnd, [ref]$rect)

    $w = $rect.Right - $rect.Left
    $h = $rect.Bottom - $rect.Top

    Write-Host "Window Bounds: $w x $h at ($($rect.Left), $($rect.Top))"

    if ($w -gt 100 -and $h -gt 100) {
        $bmp = New-Object System.Drawing.Bitmap $w, $h
        $g = [System.Drawing.Graphics]::FromImage($bmp)
        $g.CopyFromScreen($rect.Left, $rect.Top, 0, 0, (New-Object System.Drawing.Size $w, $h))
        $bmp.Save("screenshot_ringanbrowser.png", [System.Drawing.Imaging.ImageFormat]::Png)
        $g.Dispose()
        $bmp.Dispose()
        Write-Host "Screenshot saved successfully to screenshot_ringanbrowser.png"
    } else {
        # Fallback to full screen
        $screen = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
        $bmp = New-Object System.Drawing.Bitmap $screen.Width, $screen.Height
        $g = [System.Drawing.Graphics]::FromImage($bmp)
        $g.CopyFromScreen(0, 0, 0, 0, $screen.Size)
        $bmp.Save("screenshot_ringanbrowser.png", [System.Drawing.Imaging.ImageFormat]::Png)
        $g.Dispose()
        $bmp.Dispose()
        Write-Host "Fallback: Fullscreen screenshot saved to screenshot_ringanbrowser.png"
    }
} else {
    # Fullscreen fallback
    $screen = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
    $bmp = New-Object System.Drawing.Bitmap $screen.Width, $screen.Height
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.CopyFromScreen(0, 0, 0, 0, $screen.Size)
    $bmp.Save("screenshot_ringanbrowser.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose()
    $bmp.Dispose()
    Write-Host "Fullscreen screenshot saved to screenshot_ringanbrowser.png"
}
