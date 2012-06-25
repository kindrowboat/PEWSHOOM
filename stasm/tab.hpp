// $tab.hpp 3.0 milbo$
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

#include "stasm.hpp"

void AppendToTabFile(const char sTabFile[],    // in: tab file name
      const char sImageBase[],  // in: base name of image
      double StartFit,          // in: normalized to intereye dist if ME17
      double FinalFit,          // in: normalized to intereye dist if ME17
      Vec    Fits,              // in: all fits, normalized to intereye dist if ME17
      double InterEyeDist,      // in:
      double FaceWidth);        // in:

void CloseTabFile(int nImages, int  nFound, double MeanTime);
