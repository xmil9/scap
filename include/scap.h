//
// Scap API.
//
//
// Dec-2016, Michael Lindner
//
#pragma once
#include "scap_environ.h"
#ifdef _MSC_VER
// Lower warning level for external headers.
#  pragma warning(push, 3)
#endif
#include <stdexcept>
#include <string>
#ifdef _MSC_VER
// Restore warning level.
#  pragma warning(pop)
#endif


namespace scap {

// Error class for scap functionality.
class Error : public std::runtime_error {
public:
   explicit Error(std::string const& description)
      : std::runtime_error(description) {}
};


// Captures the screen contents and saves it as png file at the given path.
SCAP_API void captureScreen(std::string const& outFilePath);

} // namespace scap

