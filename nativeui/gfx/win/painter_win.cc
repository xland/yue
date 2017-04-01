// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/gfx/win/painter_win.h"

#include <memory>

#include "base/win/scoped_gdi_object.h"
#include "nativeui/gfx/geometry/rect_conversions.h"
#include "nativeui/gfx/geometry/vector2d_conversions.h"
#include "nativeui/state.h"

namespace nu {

PainterWin::PainterWin(HDC dc, float scale_factor)
    : hdc_(dc), scale_factor_(scale_factor) {
  // Initial state.
  states_.emplace(scale_factor, Color());
  // Enable using world transformation,
  ::SetGraphicsMode(hdc_, GM_ADVANCED);
}

PainterWin::~PainterWin() {
}

void PainterWin::DrawNativeTheme(NativeTheme::Part part,
                                 ControlState state,
                                 const Rect& rect,
                                 const NativeTheme::ExtraParams& extra) {
  State::GetCurrent()->GetNativeTheme()->Paint(
      part, hdc_, state, rect, extra);
}

void PainterWin::DrawFocusRect(const Rect& rect) {
  RECT r = rect.ToRECT();
  ::DrawFocusRect(hdc_, &r);
}

void PainterWin::Save() {
  states_.push(top());
  top().state = ::SaveDC(hdc_);
}

void PainterWin::Restore() {
  if (states_.size() == 1)
    return;
  ::RestoreDC(hdc_, top().state);
  states_.pop();
}

void PainterWin::ClipRect(const RectF& rect, CombineMode mode) {
  ClipPixelRect(ToEnclosingRect(ScaleRect(rect, scale_factor_)), mode);
}

void PainterWin::Translate(const Vector2dF& offset) {
  TranslatePixel(ToFlooredVector2d(ScaleVector2d(offset, scale_factor_)));
}

void PainterWin::SetColor(Color color) {
  top().color = color;
}

void PainterWin::SetLineWidth(float width) {
  top().line_width = width;
}

void PainterWin::DrawRect(const RectF& rect) {
  DrawPixelRect(ToEnclosingRect(ScaleRect(rect, scale_factor_)));
}

void PainterWin::FillRect(const RectF& rect) {
  FillPixelRect(ToEnclosingRect(ScaleRect(rect, scale_factor_)));
}

void PainterWin::DrawColoredTextWithFlags(
    const String& text, Font* font, Color color, const RectF& rect, int flags) {
  DrawColoredTextPixelWithFlags(
      text, font, color, ToEnclosingRect(ScaleRect(rect, scale_factor_)),
      flags);
}

void PainterWin::ClipPixelRect(const Rect& rect, CombineMode mode) {
  base::win::ScopedRegion region(
      ::CreateRectRgn(rect.x(), rect.y(), rect.right(), rect.bottom()));
  int clip_mode = RGN_COPY;
  if (mode == CombineMode::Intersect)
    clip_mode = RGN_AND;
  else if (mode == CombineMode::Union)
    clip_mode = RGN_OR;
  else if (mode == CombineMode::Exclude)
    clip_mode = RGN_DIFF;
  ::ExtSelectClipRgn(hdc_, region.get(), clip_mode);
}

void PainterWin::TranslatePixel(const Vector2d& offset) {
  XFORM xform = {0};
  xform.eM11 = 1.0;
  xform.eM22 = 1.0;
  xform.eDx = offset.x();
  xform.eDy = offset.y();
  ::ModifyWorldTransform(hdc_, &xform, MWT_LEFTMULTIPLY);
}

void PainterWin::DrawPixelRect(const Rect& rect) {
  Gdiplus::Pen pen(ToGdi(top().color), top().line_width);
  Gdiplus::Graphics(hdc_).DrawRectangle(&pen, ToGdi(rect));
}

void PainterWin::FillPixelRect(const Rect& rect) {
  Gdiplus::SolidBrush brush(ToGdi(top().color));
  Gdiplus::Graphics(hdc_).FillRectangle(&brush, ToGdi(rect));
}

void PainterWin::DrawColoredTextPixelWithFlags(
    const String& text, Font* font, Color color, const Rect& rect, int flags) {
  Gdiplus::SolidBrush brush(ToGdi(color));
  Gdiplus::StringFormat format;
  format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
  if (flags & TextAlignLeft)
    format.SetAlignment(Gdiplus::StringAlignmentNear);
  else if (flags & TextAlignCenter)
    format.SetAlignment(Gdiplus::StringAlignmentCenter);
  else if (flags & TextAlignRight)
    format.SetAlignment(Gdiplus::StringAlignmentFar);
  Gdiplus::Graphics(hdc_).DrawString(
      text.c_str(), static_cast<int>(text.size()),
      font->GetNative(), ToGdi(RectF(rect)), &format, &brush);
}

}  // namespace nu
