// $imgiven.cpp 3.0 milbo$ routines for finding images given directories
//
// Routines in this file use the general purpose global
// buffer sgBuf for storing error messages.
//
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

#include "stasm.hpp"

// these are the image extensions we try when searching for an image

#if _MSC_VER
const char sgExts[] = "pgm;jpg;bmp;ppm;png";
#else
const char sgExts[] = "pgm;jpg;bmp;ppm"; // png files not supported
#endif

//-----------------------------------------------------------------------------
// Return the string at index i in sStrings, where sStrings
// has the form dir1;dir2;dir3

static char *sGetNthString (const char sStrings[], int iString)
{
// following def needed because strtok replaces ; with a 0
static char sStrings1[SLEN]; strcpy(sStrings1, sStrings);

int i = 0;
char *sDir = strtok(sStrings1, ";");
while (i++ < iString && sDir != NULL)
    sDir = strtok(NULL, ";");
return sDir;
}

//-----------------------------------------------------------------------------
// Look for a readable file with basename sFile and with an extension
// in sgExts and in a directory in sDirs
//
// Return NULL on success, with path in sPath
// Return an error msg if can't find sFile in sDirs with any of sgExts
//
// We try to open files in an order that minimizes the number of unsuccessful
// openings --- we first use the directory and extension used in the previous
// call to this function because that is most likely to be correct.

const char *
sGetPathGivenDirs (
        char sPath[],               // out
        const char sBase[],         // in
        const char sDirs[],         // in: filename base (dir and ext ignored)
        const char sShapeFile[],    // in: shape file holding sDirs, for err msgs
        bool fExitOnErr)            // in
{
static char sLastDir[SLEN];
static char sLastExt[SLEN];
static bool fNewLine;               // just to get newlines right in log file

char sDrive1[_MAX_DRIVE], sDir1[_MAX_DIR], sBase1[_MAX_FNAME], sExt1[_MAX_EXT];
splitpath(sBase, sDrive1, sDir1, sBase1, sExt1);
if (sDrive1[0] || sDir1[0])
    Warn("ignoring directory of %s (tags in %s should not contain directories)",
        sBase, sShapeFile);
else if (sExt1[0])
    Warn("ignoring extension %s of %s (tags in %s should not have extensions)",
        sExt1, sBase, sShapeFile);

// first try the same dir and ext we used last time (for efficiency)

FILE *pFile = NULL;
if (sLastDir[0] && sLastExt[0])
    {
    sprintf(sPath, "%s/%s.%s", sLastDir, sBase1, sLastExt);
    logprintf("Opening %s ", sPath);
    fNewLine = true;
    pFile = fopen(sPath, "r");
    }
if (!pFile && sLastExt[0])
    {
    // no success, try the same ext we used last time but with
    // each dir in turn (once again, for efficiency)

    int iDir = 0; char *sDir;
    while (!pFile && NULL != (sDir = sGetNthString(sDirs, iDir++)))
        {
        sprintf(sPath, "%s/%s.%s", sDir, sBase1, sLastExt);
        logprintf("%sOpening %s ", (fNewLine? "unsuccessful, trying next directory\n": ""), sPath);
        fNewLine = true;
        pFile = fopen(sPath, "r");
        }
    if (pFile)  // success?
        strcpy(sLastDir, sDir);
    }
if (!pFile)
    {
    // still no success, try each dir and ext in turn

    int iExt = 0; char *sExt;
    while (!pFile && NULL != (sExt = sGetNthString(sgExts, iExt++)))
        {
        int iDir = 0; char *sDir;
        strcpy(sExt1, sExt);    // needed because sGetNthString uses strtok
        while (!pFile && NULL != (sDir = sGetNthString(sDirs, iDir++)))
            {
            sprintf(sPath, "%s/%s.%s", sDir, sBase1, sExt1);
            logprintf("%sOpening %s ", (fNewLine? "unsuccesful, trying next directory\n": ""), sPath);
            fNewLine = true;
            pFile = fopen(sPath, "r");
            }
        if (pFile)  // success?
            {
            strcpy(sLastDir, sDir);
            strcpy(sLastExt, sExt1);
            }
        }
    }
if (!pFile)
    {
    logprintf("unsuccessful\n");
    sprintf(sgBuf, "can't open %s\n"
           "       Searched in directories %s\n"
           "       With extensions %s\n"
           "       Check definition of \"Directories\" in %s?",
           sBase1, sDirs, sgExts, sShapeFile);
    if (fExitOnErr)
        Err(sgBuf);
    return sgBuf;   // return error msg
    }
logprintf("\n");
fclose(pFile);
return NULL;        // success
}

//-----------------------------------------------------------------------------
static const char *
sLoadImageGivenDirs (
        Image *pImg,             // out
        RgbImage *pRgbImg,       // out
        const char sFile[],      // in: only base name of this is used
        const char sDirs[],      // in
        const char sShapeFile[], // in: shape file holding sDirs, for err msgs only
        bool fExitOnErr,         // in
        bool fRemovePad)         // in
{
char sPath[SLEN];

const char *sErrMsg = sGetPathGivenDirs(sPath,
                                        sFile, sDirs, sShapeFile, fExitOnErr);

if (!sErrMsg)   // succesfully found path?
    sErrMsg = sLoadImage(pImg, pRgbImg, sPath, false, fRemovePad);

return sErrMsg; // typecast discards const
}

//-----------------------------------------------------------------------------
const char *
sLoadImageGivenDirs (
        Image &Img,              // out
        const char sFile[],      // in: only base name of this is used
        const char sDirs[],      // in
        const char sShapeFile[], // in: shape file holding sDirs, for err msgs only
        bool fExitOnErr)         // in
{
return sLoadImageGivenDirs(&Img, NULL,
                           sFile, sDirs, sShapeFile, fExitOnErr, true);
}

const char *
sLoadImageGivenDirs (
        RgbImage &Img,           // out
        const char sFile[],      // in: only base name of this is used
        const char sDirs[],      // in
        const char sShapeFile[], // in: shape file holding sDirs, for err msgs only
        bool fExitOnErr)         // in
{
return sLoadImageGivenDirs(NULL, &Img,
                           sFile, sDirs, sShapeFile, fExitOnErr, true);
}
