#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <cmath>

namespace VectorIcons {
    using namespace Gdiplus;

    inline void DrawBackIcon(Graphics& g, float cx, float cy, float s, Color col, bool enabled = true) {
        if (!enabled) col = Color(100, 100, 100);
        Pen pen(col, 2.0f);
        pen.SetStartCap(LineCapRound);
        pen.SetEndCap(LineCapRound);
        pen.SetLineJoin(LineJoinRound);

        float r = s * 0.4f;
        // Arrow line
        g.DrawLine(&pen, cx + r * 0.7f, cy, cx - r * 0.7f, cy);
        // Arrow heads
        g.DrawLine(&pen, cx - r * 0.7f, cy, cx - r * 0.1f, cy - r * 0.6f);
        g.DrawLine(&pen, cx - r * 0.7f, cy, cx - r * 0.1f, cy + r * 0.6f);
    }

    inline void DrawForwardIcon(Graphics& g, float cx, float cy, float s, Color col, bool enabled = true) {
        if (!enabled) col = Color(100, 100, 100);
        Pen pen(col, 2.0f);
        pen.SetStartCap(LineCapRound);
        pen.SetEndCap(LineCapRound);
        pen.SetLineJoin(LineJoinRound);

        float r = s * 0.4f;
        // Arrow line
        g.DrawLine(&pen, cx - r * 0.7f, cy, cx + r * 0.7f, cy);
        // Arrow heads
        g.DrawLine(&pen, cx + r * 0.7f, cy, cx + r * 0.1f, cy - r * 0.6f);
        g.DrawLine(&pen, cx + r * 0.7f, cy, cx + r * 0.1f, cy + r * 0.6f);
    }

    inline void DrawReloadIcon(Graphics& g, float cx, float cy, float s, Color col, bool isLoading = false) {
        Pen pen(col, 2.0f);
        pen.SetStartCap(LineCapRound);
        pen.SetEndCap(LineCapRound);

        float r = s * 0.35f;
        if (isLoading) {
            // Draw 'X' for stop
            g.DrawLine(&pen, cx - r * 0.7f, cy - r * 0.7f, cx + r * 0.7f, cy + r * 0.7f);
            g.DrawLine(&pen, cx + r * 0.7f, cy - r * 0.7f, cx - r * 0.7f, cy + r * 0.7f);
        } else {
            // Circular arc arrow
            RectF arcRect(cx - r, cy - r, r * 2.0f, r * 2.0f);
            g.DrawArc(&pen, arcRect, 45.0f, 275.0f);

            // Arrow head at top end (approx 320 degrees)
            float angleRad = 320.0f * 3.14159265f / 180.0f;
            float hx = cx + r * cosf(angleRad);
            float hy = cy + r * sinf(angleRad);
            g.DrawLine(&pen, hx, hy, hx + 4.5f, hy - 1.0f);
            g.DrawLine(&pen, hx, hy, hx + 1.0f, hy - 5.0f);
        }
    }

    inline void DrawHomeIcon(Graphics& g, float cx, float cy, float s, Color col) {
        Pen pen(col, 2.0f);
        pen.SetStartCap(LineCapRound);
        pen.SetEndCap(LineCapRound);
        pen.SetLineJoin(LineJoinRound);

        float r = s * 0.35f;
        // Roof
        PointF roof[] = {
            PointF(cx - r, cy),
            PointF(cx, cy - r),
            PointF(cx + r, cy)
        };
        g.DrawLines(&pen, roof, 3);
        // Base walls
        g.DrawLine(&pen, cx - r * 0.75f, cy, cx - r * 0.75f, cy + r);
        g.DrawLine(&pen, cx + r * 0.75f, cy, cx + r * 0.75f, cy + r);
        g.DrawLine(&pen, cx - r * 0.75f, cy + r, cx + r * 0.75f, cy + r);
        // Door
        g.DrawLine(&pen, cx - r * 0.25f, cy + r, cx - r * 0.25f, cy + r * 0.4f);
        g.DrawLine(&pen, cx + r * 0.25f, cy + r, cx + r * 0.25f, cy + r * 0.4f);
        g.DrawLine(&pen, cx - r * 0.25f, cy + r * 0.4f, cx + r * 0.25f, cy + r * 0.4f);
    }

    inline void DrawLockIcon(Graphics& g, float cx, float cy, float s, bool isSecure) {
        Color col = isSecure ? Color(255, 34, 197, 94) : Color(255, 148, 163, 184); // Green or Gray
        Pen pen(col, 1.8f);
        pen.SetStartCap(LineCapRound);
        pen.SetEndCap(LineCapRound);

        float w = s * 0.5f;
        float h = s * 0.4f;
        float yBody = cy - 1.0f;
        
        // Lock shackle (loop)
        RectF shackleRect(cx - w * 0.35f, cy - h - 1.0f, w * 0.7f, h * 1.1f);
        g.DrawArc(&pen, shackleRect, 180.0f, 180.0f);
        g.DrawLine(&pen, cx - w * 0.35f, cy - h * 0.45f, cx - w * 0.35f, yBody);
        g.DrawLine(&pen, cx + w * 0.35f, cy - h * 0.45f, cx + w * 0.35f, yBody);

        // Lock body (rounded rect)
        SolidBrush bodyBrush(col);
        GraphicsPath path;
        float bw = w * 0.9f;
        float bh = h * 0.9f;
        float bx = cx - bw * 0.5f;
        float by = yBody;
        path.AddRectangle(RectF(bx, by, bw, bh));
        g.FillPath(&bodyBrush, &path);

        // Keyhole
        SolidBrush holeBrush(Color(255, 15, 23, 42));
        g.FillEllipse(&holeBrush, cx - 1.5f, by + 2.5f, 3.0f, 3.0f);
    }

    inline void DrawStarIcon(Graphics& g, float cx, float cy, float s, bool isBookmarked) {
        Color col = isBookmarked ? Color(255, 234, 179, 8) : Color(255, 148, 163, 184); // Gold or Gray
        Pen pen(col, 1.8f);
        pen.SetLineJoin(LineJoinRound);

        PointF pts[10];
        float rOuter = s * 0.4f;
        float rInner = rOuter * 0.45f;
        for (int i = 0; i < 10; ++i) {
            float angle = (i * 36.0f - 90.0f) * 3.14159265f / 180.0f;
            float r = (i % 2 == 0) ? rOuter : rInner;
            pts[i] = PointF(cx + r * cosf(angle), cy + r * sinf(angle));
        }

        if (isBookmarked) {
            SolidBrush brush(col);
            g.FillPolygon(&brush, pts, 10);
        } else {
            g.DrawPolygon(&pen, pts, 10);
        }
    }

    inline void DrawPlusIcon(Graphics& g, float cx, float cy, float s, Color col) {
        Pen pen(col, 2.0f);
        pen.SetStartCap(LineCapRound);
        pen.SetEndCap(LineCapRound);

        float r = s * 0.35f;
        g.DrawLine(&pen, cx - r, cy, cx + r, cy);
        g.DrawLine(&pen, cx, cy - r, cx, cy + r);
    }

    inline void DrawCloseIcon(Graphics& g, float cx, float cy, float s, Color col) {
        Pen pen(col, 1.8f);
        pen.SetStartCap(LineCapRound);
        pen.SetEndCap(LineCapRound);

        float r = s * 0.3f;
        g.DrawLine(&pen, cx - r, cy - r, cx + r, cy + r);
        g.DrawLine(&pen, cx + r, cy - r, cx - r, cy + r);
    }

    inline void DrawDevToolsIcon(Graphics& g, float cx, float cy, float s, Color col) {
        Pen pen(col, 1.8f);
        pen.SetStartCap(LineCapRound);
        pen.SetEndCap(LineCapRound);
        pen.SetLineJoin(LineJoinRound);

        float r = s * 0.35f;
        // <
        PointF leftBracket[] = {
            PointF(cx - r * 0.3f, cy - r * 0.7f),
            PointF(cx - r * 0.8f, cy),
            PointF(cx - r * 0.3f, cy + r * 0.7f)
        };
        g.DrawLines(&pen, leftBracket, 3);
        // >
        PointF rightBracket[] = {
            PointF(cx + r * 0.3f, cy - r * 0.7f),
            PointF(cx + r * 0.8f, cy),
            PointF(cx + r * 0.3f, cy + r * 0.7f)
        };
        g.DrawLines(&pen, rightBracket, 3);
        // /
        g.DrawLine(&pen, cx + r * 0.2f, cy - r * 0.7f, cx - r * 0.2f, cy + r * 0.7f);
    }

    inline void DrawSettingsIcon(Graphics& g, float cx, float cy, float s, Color col) {
        Pen pen(col, 1.8f);
        float rOut = s * 0.35f;
        float rIn = rOut * 0.5f;

        // Outer gear circle
        g.DrawEllipse(&pen, cx - rOut, cy - rOut, rOut * 2.0f, rOut * 2.0f);
        // Inner hole
        g.DrawEllipse(&pen, cx - rIn, cy - rIn, rIn * 2.0f, rIn * 2.0f);

        // 6 gear cogs
        for (int i = 0; i < 6; ++i) {
            float ang = (i * 60.0f) * 3.14159265f / 180.0f;
            float x1 = cx + (rOut - 1.0f) * cosf(ang);
            float y1 = cy + (rOut - 1.0f) * sinf(ang);
            float x2 = cx + (rOut + 3.0f) * cosf(ang);
            float y2 = cy + (rOut + 3.0f) * sinf(ang);
            g.DrawLine(&pen, x1, y1, x2, y2);
        }
    }

    inline void DrawGlobeIcon(Graphics& g, float cx, float cy, float s, Color col) {
        Pen pen(col, 1.5f);
        float r = s * 0.35f;
        g.DrawEllipse(&pen, cx - r, cy - r, r * 2.0f, r * 2.0f);
        // Equator
        g.DrawLine(&pen, cx - r, cy, cx + r, cy);
        // Meridian ellipse
        g.DrawEllipse(&pen, cx - r * 0.45f, cy - r, r * 0.9f, r * 2.0f);
    }

    inline void DrawDownloadIcon(Graphics& g, float cx, float cy, float s, Color col) {
        Pen pen(col, 2.0f);
        pen.SetStartCap(LineCapRound);
        pen.SetEndCap(LineCapRound);
        pen.SetLineJoin(LineJoinRound);

        float r = s * 0.35f;
        // Arrow shaft down
        g.DrawLine(&pen, cx, cy - r * 0.8f, cx, cy + r * 0.4f);
        // Arrow head down
        g.DrawLine(&pen, cx - r * 0.5f, cy, cx, cy + r * 0.4f);
        g.DrawLine(&pen, cx + r * 0.5f, cy, cx, cy + r * 0.4f);
        // Tray / base
        PointF tray[] = {
            PointF(cx - r * 0.8f, cy + r * 0.2f),
            PointF(cx - r * 0.8f, cy + r * 0.8f),
            PointF(cx + r * 0.8f, cy + r * 0.8f),
            PointF(cx + r * 0.8f, cy + r * 0.2f)
        };
        g.DrawLines(&pen, tray, 4);
    }
}
