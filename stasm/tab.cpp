// $tab.cpp 3.0 milbo$ generate xxx.tab file (summarizes stasm results)
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

static FILE *pFile;

//-----------------------------------------------------------------------------
void AppendToTabFile (const char sTabFile[],    // in: tab file name
      const char sImageBase[],  // in: base name of image
      double StartFit,          // in: normalized to intereye dist if ME17
      double FinalFit,          // in: normalized to intereye dist if ME17
      Vec    Fits,              // in: all fits, normalized to intereye dist if ME17
      double InterEyeDist,      // in:
      double FaceWidth)         // in:
{
int iPoint;
const int nPoints = Fits.nelems();

if (!pFile)         // first time? if so, must init file
    {
    lprintf("Writing %s\n", sTabFile);
    pFile = Fopen(sTabFile, "w");   // will issue error if can't open
    Fprintf(pFile, "file                   start   final intereye facewidth");
    for (iPoint = 0; iPoint < nPoints; iPoint++)
        Fprintf(pFile, "     p%2.2d", iPoint);
    Fprintf(pFile, "\n");
    }
if (StartFit >= 0 && FinalFit >= 0)     // fits valid?
    {
    Fprintf(pFile, "%-20.20s %7.5f %7.5f  %7.3f   %7.2f",
            sImageBase, StartFit, FinalFit, InterEyeDist, FaceWidth);
    for (iPoint = 0; iPoint < nPoints; iPoint++)
        Fprintf(pFile, " %7.5f", Fits(iPoint));
    Fprintf(pFile, "\n");
    fflush(pFile);
    }
}

//-----------------------------------------------------------------------------
void CloseTabFile (int nImages, int  nFound, double MeanTime)
{
if (pFile == NULL)  // this happens if face detector fails for all image
    Warn("tab file not created");
else
    {
    Fprintf(pFile, "# nImages %d nFound %d [mean time %.3f secs]\n",
            nImages, nFound, MeanTime);
    fclose(pFile);
    pFile = NULL;
    }
}
