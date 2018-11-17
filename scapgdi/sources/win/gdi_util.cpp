//
// Dec-2016, Michael Lindner
//
#include "stdafx.h"
#include "gdi_util.h"
#include "libpng/util/libpng_util.h"
// Lower warning level for external headers.
#pragma warning(push, 3)
#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
// Restore warning level.
#pragma warning(pop)


///////////////////

namespace {

using namespace gdi;


inline size_t bytesFromBits(size_t numBits) {
   return (numBits + 7) / 8;
}


// Calculates the size of a bitmap's pixel data in bytes.
inline size_t calcBitmapSize(BITMAPINFO const& bmInfo) {
   return bmInfo.bmiHeader.biWidth * std::abs(bmInfo.bmiHeader.biHeight) *
          bytesFromBits(bmInfo.bmiHeader.biBitCount);
}


void collectBitmapData(HBITMAP hbm, BITMAPFILEHEADER* bmFileHeader,
                       BITMAPINFO* bmInfo, std::vector<uint8_t>* pixelData) {
   assert(bmFileHeader && bmInfo && pixelData);
   if (!hbm)
      return;

   WindowDC screenDc = WindowDC::createScreenDC();

   // Populate bitmap info data.
   ZeroMemory(bmInfo, sizeof(BITMAPINFO));
   bmInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   GetDIBits(screenDc.handle(), hbm, 0, 0, NULL, bmInfo, DIB_RGB_COLORS);

   if (bmInfo->bmiHeader.biSizeImage <= 0)
      bmInfo->bmiHeader.biSizeImage =
         static_cast<DWORD>(calcBitmapSize(*bmInfo));
   bmInfo->bmiHeader.biCompression = BI_RGB;

   // Populate bitmap pixel data.
   pixelData->resize(bmInfo->bmiHeader.biSizeImage, 0);
   GetDIBits(screenDc.handle(), hbm, 0,
             static_cast<UINT>(bmInfo->bmiHeader.biHeight), pixelData->data(),
             bmInfo, DIB_RGB_COLORS);

   // Populate file header.
   bmFileHeader->bfReserved1 = 0;
   bmFileHeader->bfReserved2 = 0;
   bmFileHeader->bfType = 'MB';
   bmFileHeader->bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
                          bmInfo->bmiHeader.biSizeImage;
   // Offset from beginning of file to pixel data.
   bmFileHeader->bfOffBits =
      sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
}


void saveAsBitmap(BITMAPFILEHEADER const& bmFileHeader,
                  BITMAPINFO const& bmInfo,
                  std::vector<uint8_t> const& pixelData,
                  std::string const& filePath) {
   std::ofstream file(filePath, std::ofstream::out | std::ifstream::binary);
   if (!file.is_open())
      throw gdi::Error("Failure to open destination file for bitmap.");

   file.write(reinterpret_cast<char const*>(&bmFileHeader),
              sizeof(BITMAPFILEHEADER));
   file.write(reinterpret_cast<char const*>(&bmInfo), bmInfo.bmiHeader.biSize);
   file.write(reinterpret_cast<char const*>(pixelData.data()),
              static_cast<std::streamsize>(pixelData.size()));
   file.close();
}


///////////////////

inline png_uint_32 pngFormatFromBmpFormat(BITMAPINFO const& bmInfo) {
   if (bmInfo.bmiHeader.biBitCount != 32 ||
      bmInfo.bmiHeader.biCompression != 0 || bmInfo.bmiHeader.biPlanes != 1)
      throw Error("Unsupported bitmap format for conversion to PNG.");
   return PNG_FORMAT_BGRA;
}


// Calculates the size in bytes of a row of pixel in a bitmap.
inline std::size_t calcRowSize(BITMAPINFO const& bmInfo) {
   const std::size_t bytesPerPixel =
      static_cast<std::size_t>((bmInfo.bmiHeader.biBitCount + 7) / 8);
   return static_cast<std::size_t>(bmInfo.bmiHeader.biWidth * bytesPerPixel);
}


// The bitmap format might store the image pixels upside down. This function
// turns the image upright if necessary. It returns a pointer to the upright
// image pixels. If the images needs upright-ing the passed pixel buffer will
// hold the uprighted image and the returned pointer will point to the buffer's
// data. If no upright-ing is needed then a pointer to the source pixels is
// returned.
uint8_t const* uprightBitmapImage(BITMAPINFO const& bmInfo,
                                  std::vector<uint8_t> const& srcPixels,
                                  std::vector<uint8_t>* uprightedPixelBuffer) {
   assert(uprightedPixelBuffer);
   // A negative height value indicates a normal (upright) image. The normal
   // case, however, is upside down, i.e. the image height is positive.
   const bool isUpsideDown = bmInfo.bmiHeader.biHeight >= 0;
   if (!isUpsideDown)
      return srcPixels.data();

   uprightedPixelBuffer->resize(srcPixels.size());

   const std::size_t rowSize = calcRowSize(bmInfo);
   const std::size_t numRows =
      static_cast<std::size_t>(bmInfo.bmiHeader.biHeight);
   uint8_t const* srcRow = srcPixels.data();
   uint8_t* destRow = uprightedPixelBuffer->data() + (numRows - 1) * rowSize;
   for (std::size_t rowIdx = 0; rowIdx < numRows; ++rowIdx) {
      std::copy(srcRow, srcRow + rowSize, destRow);
      srcRow += rowSize;
      destRow -= rowSize;
   }

   return uprightedPixelBuffer->data();
}


// Saves a bitmap as a PNG file.
void saveAsPng(BITMAPINFO const& bmInfo, std::vector<uint8_t> const& pixelData,
               std::string const& filePath) {
   pngutil::libpng::PngInfo imageInfo(
      pngFormatFromBmpFormat(bmInfo),
      static_cast<png_uint_32>(bmInfo.bmiHeader.biWidth),
      static_cast<png_uint_32>(std::abs(bmInfo.bmiHeader.biHeight)));

   std::vector<uint8_t> uprightPixelBuffer;
   uint8_t const* uprightPixels =
      uprightBitmapImage(bmInfo, pixelData, &uprightPixelBuffer);

   pngutil::libpng::savePng(
      filePath, imageInfo, uprightPixels,
      static_cast<png_uint_32>(bmInfo.bmiHeader.biSizeImage));
}

} // namespace


namespace gdi {

///////////////////

WindowDC::WindowDC(HWND wnd_)
   : WindowDC(wnd_, GetDC(wnd)) {
   if (!hdc)
      throw Error("GDI: Failure to acquire DC of window.");
}


WindowDC::WindowDC(HWND wnd_, HDC hdc_)
   : wnd(wnd_), hdc(hdc_) {
   if (!hdc)
      throw Error("GDI: Failure to acquire DC of window.");
}


WindowDC::~WindowDC() {
   if (hdc)
      ReleaseDC(wnd, hdc);
}


///////////////////

CompatibleDC::CompatibleDC(HDC srcDc)
   : hdc(CreateCompatibleDC(srcDc)) {
   if (!hdc)
      throw Error("GDI: Failure to create a compatible DC.");
}


CompatibleDC::~CompatibleDC() {
   if (hdc)
      DeleteDC(hdc);
}


///////////////////

Bitmap::Bitmap(HBITMAP hbm_)
   : hbm(hbm_) {
}


Bitmap::~Bitmap() {
   if (hbm)
      DeleteObject(hbm);
}


///////////////////

void	saveBitmap(HBITMAP hbm, std::string const& filePath) {
   if (!hbm || filePath.empty())
      return;

   BITMAPFILEHEADER bmFileHeader;
   BITMAPINFO bmInfo;
   std::vector<uint8_t> pixelData;
   collectBitmapData(hbm, &bmFileHeader, &bmInfo, &pixelData);

   saveAsBitmap(bmFileHeader, bmInfo, pixelData, filePath);
}


void saveBitmapAsPng(HBITMAP hbm, std::string const& filePath) {
   if (!hbm || filePath.empty())
      return;

   BITMAPFILEHEADER bmFileHeader;
   BITMAPINFO bmInfo;
   std::vector<uint8_t> pixelData;
   collectBitmapData(hbm, &bmFileHeader, &bmInfo, &pixelData);

   saveAsPng(bmInfo, pixelData, filePath);
}

} // namespace gdi