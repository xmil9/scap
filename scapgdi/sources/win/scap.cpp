//
// Dec-2016, Michael Lindner
//
#include "stdafx.h"
#include "scap.h"
#include "gdi_util.h"
// Lower warning level for external headers.
#pragma warning(push, 3)
#include <cassert>
// Restore warning level.
#pragma warning(pop)


namespace scap {

SCAP_API void captureScreen(std::string const& outFilePath) {
   const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
   const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

   gdi::WindowDC desktopDc(GetDesktopWindow());
   gdi::CompatibleDC captureDc(desktopDc.handle());
   gdi::Bitmap capturedBitmap(
      CreateCompatibleBitmap(desktopDc.handle(), screenWidth, screenHeight));
   SelectObject(captureDc.handle(), capturedBitmap.handle());
   BitBlt(captureDc.handle(), 0, 0, screenWidth, screenHeight,
          desktopDc.handle(), 0, 0, SRCCOPY | CAPTUREBLT);

   //gdi::saveBitmap(capturedBitmap.handle(), outFilePath);
   gdi::saveBitmapAsPng(capturedBitmap.handle(), outFilePath);
}

} //namespace scap