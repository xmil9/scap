//
// Scap implementation for Linux using X11.
//
// Jan-2017, Michael Lindner
//
#include "scap.h"
#include "libpng/util/libpng_util.h"
#include "x_util.h"
#include <X11/Xlib.h>
#include <X11/X.h>
#include <iostream>


namespace {

png_uint_32 pngFormatFromXImage(XImage* image) {
   if (image->format == ZPixmap && image->bits_per_pixel == 32)
      return PNG_FORMAT_RGBA;
   else
      throw scap::Error("Unsupported XImage format.");
}


std::size_t calcXImageDataSize(XImage* image) {
   return image->bytes_per_line * image->height;
}


void saveAsPng(XImage* image, std::string const& file) {
   pngutil::libpng::PngInfo pngInfo(pngFormatFromXImage(image), image->width,
                                    image->height);
   pngutil::libpng::savePng(file, pngInfo,
                            reinterpret_cast<png_byte*>(image->data),
                            calcXImageDataSize(image));
}


void captureScreenX(std::string const& outFilePath) {
   x::ScopedDisplay display(XOpenDisplay(nullptr));
   if (!display)
      throw scap::Error("Failure to open X display.");
   
   Window rootWnd = DefaultRootWindow(display.get());
   XWindowAttributes wndAttribs;
   XGetWindowAttributes(display.get(), rootWnd, &wndAttribs);
   int width = wndAttribs.width;
   int height = wndAttribs.height;

   x::ScopedImage screenshot(XGetImage(display.get(), rootWnd, 0, 0, width,
                                       height, AllPlanes, ZPixmap));
   if (!screenshot)
      throw scap::Error("Failure to get display image.");

   saveAsPng(screenshot.get(), outFilePath);
}

} // namespace


namespace scap {

SCAP_API void captureScreen(std::string const& outFilePath) {
   if (outFilePath.empty())
      throw scap::Error("Invalid argument passed.");

   try {
      captureScreenX(outFilePath);
   } catch (std::runtime_error& ex) {
      throw scap::Error(ex.what());
   }
}


} // namespace scap
