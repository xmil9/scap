//
// Dec-2016, Michael Lindner
//
#pragma once


namespace gdi {

///////////////////

inline HDC WindowDC::handle() const {
   return hdc;
}


inline WindowDC WindowDC::createScreenDC() {
   return WindowDC(NULL, GetDC(NULL));
}


///////////////////

inline HDC CompatibleDC::handle() const {
   return hdc;
}


///////////////////

inline HBITMAP Bitmap::handle() const {
   return hbm;
}


} // namespace gdi