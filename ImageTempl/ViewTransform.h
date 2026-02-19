// ViewTransform.h - simple view transform owner for zoom & pan
#pragma once
#include <afxwin.h>
#include <algorithm>

#ifdef min
#undef min
#endif

struct CPoint2d { double x; double y; };

class ViewTransform {
public:
    ViewTransform() { scale = 1.0; offset.x = 0.0; offset.y = 0.0; }

    double GetScale() const { return scale; }
	void SetScale(double newScale) { scale = newScale; }
    CPoint2d GetOffset() const { return offset; }
    void SetOffset(const CPoint2d& newOffset) { offset = newOffset; }

    // Zoom around a client point (in client/viewport coordinates). factor >1 zooms in.
    // The world point under the cursor remains under the cursor after zoom.
    void ZoomAt(const CPoint& clientPt, double factor) {
        double newScale = scale * factor;
        // Get world coordinate of the point BEFORE zoom using current scale/offset
        double wx = (clientPt.x - offset.x) / scale;
        double wy = (clientPt.y - offset.y) / scale;
        // Adjust offset so the same world point stays under the cursor AFTER zoom
        // offset_new = clientPt - wx * newScale
        offset.x = clientPt.x - wx * newScale;
        offset.y = clientPt.y - wy * newScale;
        scale = newScale;
    }

    // Alternative: explicit client coordinate zoom (safer for windowed views)
    void ZoomAtClient(const CPoint& clientPt, double factor, CWnd* pWnd = nullptr) {
        double newScale = scale * factor;
        double wx = (clientPt.x - offset.x) / scale;
        double wy = (clientPt.y - offset.y) / scale;
        offset.x = clientPt.x - wx * newScale;
        offset.y = clientPt.y - wy * newScale;
        scale = newScale;
    }

    // Zoom to fit image rect into client rect, then center
    void ZoomToFit(const CRect& imageRect, const CRect& clientRect) {
        if (imageRect.IsRectEmpty() || clientRect.IsRectEmpty()) return;
        
        // Calculate scale to fit image in client area (with small margin)
        double scaleX = (clientRect.Width() - 20.0) / imageRect.Width();
        double scaleY = (clientRect.Height() - 20.0) / imageRect.Height();
        scale = std::min(scaleX, scaleY);
        if (scale < 0.02) scale = 0.02;
        if (scale > 22.0) scale = 22.0;

        // Center the scaled image
        double scaledW = imageRect.Width() * scale;
        double scaledH = imageRect.Height() * scale;
        offset.x = (clientRect.Width() - scaledW) / 2.0;
        offset.y = (clientRect.Height() - scaledH) / 2.0;
    }

    // Pan by screen delta (client pixels)
    void PanBy(const CPoint& deltaScreen) {
        offset.x += deltaScreen.x;
        offset.y += deltaScreen.y;
    }

    // Convert screen (client) point to world (document) coordinates
    CPoint ScreenToWorld(const CPoint& pt) const {
        CPoint2d w;
        w.x = (pt.x - offset.x) / scale;
        w.y = (pt.y - offset.y) / scale;
        return CPoint((int)floor(w.x + 0.5), (int)floor(w.y + 0.5));
    }

    // Convert world (document) to screen (client)
    CPoint WorldToScreen(const CPoint2d& wpt) const {
        double sx = wpt.x * scale + offset.x;
        double sy = wpt.y * scale + offset.y;
        return CPoint((int)floor(sx + 0.5), (int)floor(sy + 0.5));
    }

private:
    double scale;
    CPoint2d offset; // offset in screen pixels
};
