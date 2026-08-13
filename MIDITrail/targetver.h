#pragma once

// The following macros define the minimum required platform. The minimum required
// platform is the earliest version of Windows, Internet Explorer, etc. that has the
// necessary features to run the application. The macros work by enabling all
// features available on platforms of the specified version and earlier.

// If you must specify the target platform before the definitions below, modify
// the following definitions. For the latest information on values corresponding
// to different platforms, see MSDN.
#ifndef WINVER                          // Specifies minimum required platform is Windows 10.
#define WINVER 0x0A00           // Windows 10 (DX11 + PMv2 DPI requirement)
#endif

#ifndef _WIN32_WINNT            // Specifies minimum required platform is Windows 10.
#define _WIN32_WINNT 0x0A00     // Windows 10 (DX11 + PMv2 DPI requirement)
#endif

#ifndef _WIN32_WINDOWS          // Specifies minimum required platform is Windows 98.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value for Windows Me or later.
#endif

#ifndef _WIN32_IE                       // Specifies minimum required platform is Internet Explorer 7.0.
#define _WIN32_IE 0x0700        // Change this to the appropriate value for other versions of IE.
#endif
