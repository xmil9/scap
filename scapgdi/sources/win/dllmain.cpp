//
// DLL entry point.
//
//
// Dec-2016, Michael Lindner
//
#include "stdafx.h"

#ifdef SCAP_DLL

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD reason_for_call,
                      LPVOID /*lpReserved*/
                      ) {
   switch (reason_for_call) {
   case DLL_PROCESS_ATTACH:
   case DLL_THREAD_ATTACH:
   case DLL_THREAD_DETACH:
   case DLL_PROCESS_DETACH:
      break;
   }
   return TRUE;
}

#endif // SCAP_DLL