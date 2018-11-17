//
// DirectX utilities inlined funtionality.
//
//
// Jan-2017, Michael Lindner
//
#pragma once

namespace dx {

///////////////////

inline Error::Error(const std::string& msg)
   : std::runtime_error(msg)
{}


///////////////////

inline MappedSurfaceData::~MappedSurfaceData() {
   if (m_surf)
      m_surf->Unmap();
}


inline INT MappedSurfaceData::pitch() const {
   return m_data.Pitch;
}


inline BYTE* MappedSurfaceData::bits() const {
   return m_data.pBits;
}

} // namespace dx
