// $winfile.cpp 3.0 milbo$ routines for opening and saving files under windows
//
// By keeping separate saved paths for reading and writing files,
// we keep distinct the default directories offered in the windows dialog
// for reading and writing files.

#include <windows.h>
#include <commctrl.h>
#include "stasm.hpp"
#include "winfile.hpp"

//-----------------------------------------------------------------------------
void GetImageFileName (char sName[], // out: get filename from user
                       HWND hWnd)    // in
{
static char sPath[SLEN];
OPENFILENAME ofn;
ZeroMemory(&ofn, sizeof(ofn));
ofn.lStructSize = sizeof(ofn);
ofn.hwndOwner = hWnd;
ofn.lpstrFile = sPath;
ofn.nMaxFile = sizeof(sPath);
ofn.lpstrFilter = "Images\0*.jpg;*.bmp;*.png;*.pgm;*.ppm\0All\0*.*\0";
ofn.nFilterIndex = 1;
ofn.lpstrFileTitle = NULL;
ofn.nMaxFileTitle = 0;
ofn.lpstrInitialDir = NULL;
ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
sName[0] = 0;
if (GetOpenFileName(&ofn))  // displays open file dialog box
    strcpy(sName, ofn.lpstrFile);
}

//-----------------------------------------------------------------------------
// Get a bmp filename from the user.

void GetSaveBmpFilename (char sName[],           // out: filename got from user
                         HWND hWnd)              // in
{
static char sPath[SLEN];
static char sOldTemplate[SLEN];
OPENFILENAME ofn;
ZeroMemory(&ofn, sizeof(ofn));
ofn.lStructSize = sizeof(ofn);
ofn.hwndOwner = hWnd;
ofn.lpstrFile = sPath;
ofn.nMaxFile = sizeof(sPath);
ofn.lpstrFilter = "BMP\0*.bmp\0All\0*.*\0";
ofn.nFilterIndex = 1;
ofn.lpstrFileTitle = NULL;
ofn.nMaxFileTitle = 0;
ofn.lpstrInitialDir = NULL;
ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
            OFN_OVERWRITEPROMPT | OFN_NOREADONLYRETURN;
ofn.lpstrDefExt = "bmp";
sName[0] = 0;
if (!GetSaveFileName(&ofn))  // displays open file dialog box
    {
    DWORD nErr = CommDlgExtendedError();
    if (nErr)
        Err("could not create %s", sName);
    }
else
    strcpy(sName, ofn.lpstrFile);
}
