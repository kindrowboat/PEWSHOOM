// $release.cpp 3.0 milbo$ release processor, for command line apps
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

// A non-zero nSleep time makes the overall system more reponsive when
// searching many images but slows down the stasm search slightly.
// You can change the SleepTime in the stasm .conf file.
// On my 1.6G laptop, with a nSleep=0, mean search time with the
// VJ detector is .390, compared to .354 with nSleep=0
// i.e. about 10% slower.
// With the Rowley detector, there is about a 5% difference.

int CONF_nSleep = 10;   // in milliseconds

// This is called from within long loops (e.g. in AsmSearch) so other
// applications can remain responsive e.g. so you can surf the web
// while waiting for tasm or stasm to complete :)

void ReleaseProcessor (void)
{
// TODO add support for gcc compiler

#if _MSC_VER // microsoft compiler

Sleep(CONF_nSleep);

#endif
}
