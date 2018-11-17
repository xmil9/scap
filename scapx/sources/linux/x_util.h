//
// Utilities for X11.
//
// Jan-2017, Michael Lindner
//
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>

namespace x {

///////////////////

class ScopedDisplay {
public:
   explicit ScopedDisplay(Display* d);
   ~ScopedDisplay();

   Display* get() const;
   explicit operator bool() const;

private:
   Display* display;
};




///////////////////

class ScopedImage {
public:
   explicit ScopedImage(XImage* img);
   ~ScopedImage();

   XImage* get() const;
   explicit operator bool() const;

private:
   XImage* image;
};

} // namespace x

#include "x_util.inl.h"
