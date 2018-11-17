//
// Inlined implementations for X11 utilities.
//
// Jan-2017, Michael Lindner
//

namespace x {

///////////////////

inline ScopedDisplay::ScopedDisplay(Display* d)
: display(d) {
}


inline ScopedDisplay::~ScopedDisplay() {
   if (display)
      XCloseDisplay(display);
}


inline Display* ScopedDisplay::get() const {
   return display;
}


inline ScopedDisplay::operator bool() const {
   return !!display;
}


///////////////////

inline ScopedImage::ScopedImage(XImage* img)
: image(img) {
}


inline ScopedImage::~ScopedImage() {
   if (image)
      XDestroyImage(image);
}


inline XImage* ScopedImage::get() const {
   return image;
}


inline ScopedImage::operator bool() const {
   return !!image;
}

} // namespace x
