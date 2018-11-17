//
// Dec-2016, Michael Lindner
//
#pragma once


#ifdef _MSC_VER
#  ifdef SCAP_DLL
#    ifdef SCAP_EXPORTS
#       define SCAP_API __declspec(dllexport)
#    else
#       define SCAP_API __declspec(dllimport)
#    endif // SCAP_EXPORTS
#  else
#    define SCAP_API
#  endif // SCAP_DLL
#else
#  define SCAP_API
#endif // _MSC_VER
