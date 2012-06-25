// $release.cpp 3.0 milbo$ release processor, for Windows apps
//
// If you are linking the "stasm" code into an existing application you
// will probably have to tweak some routines in this file.
//-----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// A copy of the GNU General Public License is available at
// http://www.r-project.org/Licenses/
//-----------------------------------------------------------------------------

#ifdef WIN32
#include <windows.h>
#endif
#include "stasm.hpp"

// see release.cpp for comments

int CONF_nSleep = 10;   // in milliseconds

void ReleaseProcessor (void)
{
MSG msg;
while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    }
}
