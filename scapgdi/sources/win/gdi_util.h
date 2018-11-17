//
// Utilities for GDI programming.
//
//
// Dec-2016, Michael Lindner
//
#pragma once
// Lower warning level for external headers.
#pragma warning(push, 3)
#include <windows.h>
#include <stdexcept>
#include <string>
// Restore warning level.
#pragma warning(pop)


namespace gdi {

///////////////////

// Error class for GDI utilities.
class Error : public std::runtime_error {
public:
   explicit Error(std::string const& description)
      : std::runtime_error(description) {}
};


///////////////////

// Acquires and releases the device context for a given window.
class WindowDC {
public:
   explicit WindowDC(HWND wnd_);
   WindowDC(HWND wnd_, HDC hdc_);
   ~WindowDC();
   WindowDC(WindowDC const&) = delete;
   WindowDC& operator=(WindowDC const&) = delete;
   WindowDC(WindowDC&&) = default;
   WindowDC& operator=(WindowDC&&) = default;

   static WindowDC createScreenDC();

   HDC handle() const;

private:
   HWND wnd;
   HDC hdc;
};


///////////////////

// Creates and deletes a device context created from and compatible to a given
// device context.
class CompatibleDC {
public:
   explicit CompatibleDC(HDC srcDc);
   ~CompatibleDC();
   CompatibleDC(CompatibleDC const&) = delete;
   CompatibleDC& operator=(CompatibleDC const&) = delete;
   CompatibleDC(CompatibleDC&&) = default;
   CompatibleDC& operator=(CompatibleDC&&) = default;

   HDC handle() const;

private:
   HDC hdc;
};


///////////////////

// RAII class for HBITMAP handles.
class Bitmap {
public:
   explicit Bitmap(HBITMAP hbm_);
   ~Bitmap();
   Bitmap(Bitmap const&) = delete;
   Bitmap& operator=(Bitmap const&) = delete;
   Bitmap(Bitmap&&) = default;
   Bitmap& operator=(Bitmap&&) = default;

   HBITMAP handle() const;

private:
   HBITMAP hbm;
};

// Saves a given bitmap to a .bmp file.
void saveBitmap(HBITMAP hbm, std::string const& filePath);
// Saves a given bitmap to a PNG file.
void saveBitmapAsPng(HBITMAP hbm, std::string const& filePath);

} // namespace gdi


#include "gdi_util.inl.h"
