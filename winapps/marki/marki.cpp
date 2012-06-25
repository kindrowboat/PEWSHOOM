// $marki.cpp 3.0 milbo$ manually landmark images
//
// Example:
// The following command line
//
//     marki -o new.shape -l 7 -P 2 2 ../data/muct68.shape
//
// will load all files with glasses in ../data/muct68.shape into Marki.
// If you click on an image you will set the position of landmark 7 (which
// is the tip of the chin).
// If you make changes and save the results from within Marki, you will
// _overwrite_ the shape file specified on the command line.
// Use the -o flag if you want to save the results to a different file.
//
// -l 7    specifies landmark 7 i.e. left clicking on the image will update point 7
//
// -p " m" specifies files with tag strings matching the regular exprssion
//         i.e. all XM2VTS files (look at muct68.shape and other shape files to see the tag strings)
//         -p " m[^r]" will show all except "reversed" i.e mirrored XM2VTS files
//
// -P 2 2  specifies tags with bit 0x2 set i.e. faces with glasses.
//         "-P 2 0" will show all faces _without_ glasses
//         (look at muct68.shape and atface.hpp to learn about tags)
//
// Left clicking changes the position of the current landmark.
// You can tell Marki to move automatically to the next image after every left
// click by selecting AutoNextImage (use the AutoNextImage button).
// You can tell Marki to move automatically to the next landmark after every left
// click by selecting AutoNextPoint (use the AutoNextPoint button).
// Have a look at the WmCommand function below to see what commands are available
// using the mouse.
// See the WmKeydown function below to see what you can do from the keyboard.
// Page keys, arrows,  etc. are supported
//
// Note that Marki writes to the registry (to store page layout etc. information).
//
//-----------------------------------------------------------------------------
// My windows knowledges comes from Petzold's books, so that's where
// you want to look to understand this program, if necessary.
//
// This file uses a form of Hungarian notation -- the prefix "g" on
// an identifier means "global" for example.
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

#if _WIN32_WINNT < 0x0500
#define _WIN32_WINNT 0x0500   // for WM_MOUSEWHEEL
#endif

#include <windows.h>
#include <commctrl.h>
#include <winuser.h>
#include "stasm.hpp"
#include "marki.hpp"
#include "imequalize.hpp"
#include "winfile.hpp"

static const char   *MARKI_VERSION   = "version 3.0";
       const char   *sgProgramName   = "Marki";
static const char   *sgMarkiLog      = "marki.log";
static const char   *sgClass         = "Marki";
static const char   *sgRegistryKey   = "Software\\Marki";
static const char   *sgRegistryName  = "Config";
static const int    ngDlgWidth  = 180;
static const int    nDlgHeight = 190;
static const int    ngButtonHeight = 0; // height of button bar (TODO do this properly, should be 28)

static HWND         hgMainWnd;                  // main window
static HWND         hgToolbar;                  // toolbar
static HWND         hgDlgWnd;                   // dialog window
static HBITMAP      hgToolbarBmp;               // toolbar bitmap
static HINSTANCE    hgAppInstance;              // application instance
static int          ngMainWidth, ngMainHeight;  // main window dimensions
static clock_t      gStartTime;                 // time program started
static int          xgDlg;                      // posn of dialog window
static int          ygDlg;

// command line flags

static bool         fgAllButtons;               // -B
static bool         fgUseRegEntries = true;     // -F
static bool         fgViewMode;                 // -V
static int          igLandmark = -1;            // -l
static int          igStartLandmark = -1;       // -L
static char         sgNewShapeFile[SLEN];       // -o
static char         sgTagRegex[10000];          // -p
static unsigned     gMask0, gMask1;             // -P
static bool         fgQuiet;                    // -Q
static bool         fgSlowMachine;              // -S
static bool         fgTagMode;                  // -t
static unsigned     gAttrMask;                  // -t
static char         sgFilterDir[SLEN];          // -d

// toolbar buttons

static bool         fgLock;
static bool         fgEqualize;
static bool         fgConnectDots = false;
static bool         fgShowNbrs;
static bool         fgShowCurrentNbr = true;
static bool         fgHideOriginalLandmarks;
static bool         fgCrop = true;
static int          igZoom = 4;
static bool         fgAutoNextImage;
static bool         fgAutoNextPoint;   // requires -B
static bool         fgJumpToLandmark;  // requires -B

static bool         fgMarkedFilesUpToDate = true;
static bool         fgDetShape;        // current shape is face detector shape?
static bool         fgControlKeyDown;  // set when control key is down
static bool         fgLMouseDown;      // true while left mouse button is down
static int          xgMouse, ygMouse;  // mouse position
static double       gEqAmount = 1;

static RgbImage     gImg;              // current image
static int          ngImgWidth, ngImgHeight;
static char         sgShapeFile[SLEN];

static vec_SHAPE    gShapesOrg;  // original training shapes
                                 // array[nShapes] of SHAPE
                                 // each SHAPE is nPoints x 2

static vec_SHAPE    gShapes;     // ditto, but modified by user mouse clicks

static int          ngPoints;    // number of landmarks in each training set shape

static vec_string   gTags;       // array[nShapes] of string preceding each shape
                                 // in .asm file, i.e. image filenames

static int          ngImages;           // total number of images
static unsigned     igImage = -1;       // index of current image in gTags
static char         sgImageDirs[SLEN];  // director(ies) holding images
static char         sgSavedImage[SLEN]; // filename for IDM_WriteImage button

static RgbImage     gPreloadedImage;
static int          igPreloadedImage = -1;

static int igTopCrop, igBottomCrop;  // what we crop off image before displaying it
static int igLeftCrop, igRightCrop;

static TBBUTTON gToolbarButtons[] =
    {
     6, IDM_Save,               BUTTON_Standard, // change WmNotify if you move this
    18, IDM_Lock,               BUTTON_Standard,
    29, IDM_Blank,              BUTTON_Standard,
    15, IDM_Undo,               BUTTON_Standard,
    16, IDM_Redo,               BUTTON_Standard,
    29, IDM_Blank,              BUTTON_Standard,
     7, IDM_Equalize,           BUTTON_Standard,
     8, IDM_ConnectDots,        BUTTON_Standard,
    10, IDM_ShowNbrs,           BUTTON_Standard,
    27, IDM_ShowCurrentNbr,     BUTTON_Standard,
    28, IDM_HideLandmarks,      BUTTON_Standard,
    14, IDM_WriteImage,         BUTTON_Standard,
    29, IDM_Blank,              BUTTON_Standard,
     5, IDM_Crop,               BUTTON_Standard,
     3, IDM_ZoomOut,            BUTTON_Standard,
     4, IDM_ZoomIn,             BUTTON_Standard,
    29, IDM_Blank,              BUTTON_Standard,
     2, IDM_AutoNextImage,      BUTTON_Standard,
     0, IDM_PrevImage,          BUTTON_Standard,
     1, IDM_NextImage,          BUTTON_Standard,
    29, IDM_Blank,              BUTTON_Standard,
    17, IDM_Help,               BUTTON_Standard,
    -1, // -1 terminates list
    };

static TBBUTTON gToolbarButtonsAll[] = // use if -B flag is in effect
    {
     6, IDM_Save,               BUTTON_Standard, // change WmNotify if you move this
    18, IDM_Lock,               BUTTON_Standard,
    29, IDM_Blank,              BUTTON_Standard,
    15, IDM_Undo,               BUTTON_Standard,
    16, IDM_Redo,               BUTTON_Standard,
    29, IDM_Blank,              BUTTON_Standard,
     7, IDM_Equalize,           BUTTON_Standard,
     8, IDM_ConnectDots,        BUTTON_Standard,
    10, IDM_ShowNbrs,           BUTTON_Standard,
    27, IDM_ShowCurrentNbr,     BUTTON_Standard,
    28, IDM_HideLandmarks,      BUTTON_Standard,
    14, IDM_WriteImage,         BUTTON_Standard,
    29, IDM_Blank,              BUTTON_Standard,
     5, IDM_Crop,               BUTTON_Standard,
     3, IDM_ZoomOut,            BUTTON_Standard,
     4, IDM_ZoomIn,             BUTTON_Standard,
    29, IDM_Blank,              BUTTON_Standard,
     2, IDM_AutoNextImage,      BUTTON_Standard,
     0, IDM_PrevImage,          BUTTON_Standard,
     1, IDM_NextImage,          BUTTON_Standard,
    29, IDM_Blank,              BUTTON_Standard,
    11, IDM_AutoNextPoint,      BUTTON_Standard,
    12, IDM_PrevPoint,          BUTTON_Standard,
    13, IDM_NextPoint,          BUTTON_Standard,
    30, IDM_JumpToLandmark,     BUTTON_Standard,
    29, IDM_Blank,              BUTTON_Standard,
    17, IDM_Help,               BUTTON_Standard,
    -1, // -1 terminates list
    };

static char *sgTooltips[] = // order like numbers of IDM_Save etc. in marki.hpp
    {
    "Save landmarks to shape file",                           // IDM_Save
    "Lock (disables left mouse click)",                       // IDM_Lock
    "Undo",                                                   // IDM_Undo
    "Redo",                                                   // IDM_Redo
#if 0 // TODO crashes on Vista
    "Equalize on-screen image\n\n"
        "Keyboard e (or right mouse click)\n\n"
        "Use [ and ] keys to change amount of equalization",  // IDM_Equalize
#else
    "Equalize on-screen image\n"
        "Use [ and ] keys to change amount of equalization",  // IDM_Equalize
#endif
    "Connect the dots\n\nKeyboard d",                         // IDM_ConnectDots
    "Show landmark numbers",                                  // IDM_ShowNbrs
    "Show current landmark number\n\nKeyboard n",             // IDM_ShowCurrentNbr
    "Hide original landmarks\n\nKeyboard h",                  // IDM_HideLandmarks
    "Write image to disk (will create a new filename)",       // IDM_WriteImage
    "Crop on-screen image\n\nKeyboard c",                     // IDM_Crop
    "Zoom out\n\nKeyboard -",                                 // IDM_ZoomOut
    "Zoom in\n\nKeyboard +",                                  // IDM_ZoomIn
    "Auto next image after left mouse click",                 // IDM_AutoNextImage
    "Previous image\n\nKeyboard PageUp",                      // IDM_PrevImage
    "Next image\n\nKeyboard SPACE or PageDown",               // IDM_NextImage
    "Auto next landmark after left mouse click",              // IDM_AutoNextPoint
    "Previous landmark",                                      // IDM_PrevPoint
    "Next landmark",                                          // IDM_NextPoint
    "Auto jump to landmark (automatically select current landmark)",
                                                              // IDM_JumpToLandmark
    "Help",                                                   // IDM_Help
    "",                                                       // IDM_Blank
    };

static char sgUsage[] =
"Usage: marki [FLAGS] FILE.shape\n"
"\n"
"Example: marki -o temp.shape -p \" i[^r]\" ../data/muct68.shape\n"
"\n"
"-B\t\tShow all buttons\n"
"\t\tAllows you to change the current landmark number by clicking on buttons\n"
"\n"
"-F\t\tFresh start (ignore saved registry entries for window positions etc.)\n"
"\n"
"-l LANDMARK\tLandmark to be altered by left mouse click\n"
"\t\tDefault: landmark used last time\n"
"\n"
"-L LANDMARK\tJump to this landmark whenever the image changes\n"
"\n"
"-o OUT.shape\tOutput filename\n"
"\t\tDefault: FILE.shape (i.e. overwrite the original file)\n"
"\n"
"-p PATTERN\tPATTERN is an egrep style pattern (not a file wildcard)\n"
"\t\tLoad only shapes in FILE.shape with tags matching case-independent PATTERN\n"
"\t\tExample: -p \"xyz\" loads filenames containing xyz\n"
"\t\tExample: -p \" i000| i001\" loads filenames beginning with i000 or i001\n"
"\t\tDefault: all (except face detector shapes)\n"
"\n"
"-P Mask0 Mask1\tLoad only shapes which satisfy (Attr & Mask0) == Mask1\n"
"\t\tMask1=ffffffff is treated specially, meaning satisfy (Attr & Mask0) != 0\n"
"\t\tAttr is the hex number at start of the tag, Mask0 and Mask1 are hex\n"
"\t\tThis filter is applied after -p PATTERN\n"
"\t\tExample: -P 2 2 matches faces with glasses (FA_Glasses=2, see atface.hpp)\n"
"\t\tExample: -P 2 0 matches faces without glasses\n"
"\t\tDefault: no filter (Mask0=Mask1=0)\n"
"\n"
"-d DIRECTORY\n"
"\t\tLoad only files that appear in the given diretory\n"
"\t\tThis acts as a further filter to that specified in -p and -P\n"
"\n"
"-t Mask\t\tTag mode\n"
"\t\tLeft click clears Mask bit, right click sets it\n"
"\n"
"-Q\t\tQuiet (no sound when wrapping from last image to first)\n"
"\n"
"-S\t\tSlow machine (Marki will be faster but some operations are not allowed\n"
"\n"
"-V\t\tView mode (no landmarks changes allowed, but allows you to run Marki twice at the same time)";

static const char sgHelp[] =
    "LeftMouse\tchange position of landmark\n"
    "RightMouse\ttoggle equalize (and disable \"auto jump to landmark\")\n"
    "ScrollMouse\tnext/previous image\n"
    "\n"
    "SPACE\t\tnext image\n"
    "PgDown\t\tnext image\t| PgUp\t\tprevious image\n"
    "Home\t\tfirst image\t| End\t\tlast image\n"
    "\n"
    "RightArrow\tprevious landmark\t| UpArrow\tprevious landmark\n"
    "DownArrow\tnext landmark\t| LeftArrow\tnext landmark\n"
    "\n"
    "Hold Ctrl at same time to repeat above commands 10 times\n"
    "\n"
    "\n"
    "c\tcrop image\n"
    "+ and -\tzoom cropped image\n"
    "e\tequalize image\n"
    "[ and ]\tchange amount of equalization\n"
    "d\tconnect the dots\n"
    "n\tshow current landmark number\n"
    "h\thide original landmarks\n"
    "x\tmark point as unused\n"
    "Ctrl-Z\ttoggle undo/redo\n";

static void MsgBox(char *pArgs, ...);
static void UpdateLandmark(int iImage, int iLandmark, double x, double y, bool fIsUndo);
static void WmPaint(HWND hWnd);
static void SetLandmark(int iLandmark);
static void Redisplay(void);
static void IdmNextImage(void);
static void DisplayButtons(void);
static void DisplayTitle(double xMouse = -10000, double yMouse = 0);
static void TagCurrentShape(bool fTag);

//-----------------------------------------------------------------------------
static bool fBool (int f)
{
return f == 0 || f == 1;
}

//-----------------------------------------------------------------------------
// Create the same window layout as last time by looking at the registry.
// Also get toolbar button settings.
// Init to defaults if nothing in registry or global flag fgUseRegEntries==false

static void GetStateFromRegistry (int *pxPos, int *pyPos,
                int *pxSize, int *pySize, int *pxDlg, int *pyDlg)
{
HKEY  hKey;
DWORD nBuf = SLEN;
BYTE  Buf[SLEN];
int xPos, yPos, xSize, ySize;
char sPrevShapeFile[SLEN]; sPrevShapeFile[0] = 0;
int iPrevImage = -1, iPrevLandmark = -1, iZoom = -1;
// we use int instead of bool for these because you can't scanf a bool
int fEqualize, fCrop, fConnectDots, fShowNbrs, fShowCurrentNbr, fHideOriginalLandmarks;
int fAutoNextImage, fAutoNextPoint, fJumpToLandmark;
float EqAmount;
int xDlg = -1, yDlg = -1;
bool fUseDefaults = !fgUseRegEntries;

// get values from registry and validate them

if (fgUseRegEntries)
    {
    if (ERROR_SUCCESS !=
            RegOpenKeyEx(HKEY_CURRENT_USER, sgRegistryKey, 0, KEY_READ, &hKey) ||
        ERROR_SUCCESS !=
            RegQueryValueEx(hKey, sgRegistryName, NULL, NULL, Buf, &nBuf))
        {
        logprintf(
            "Can't read registry HKEY_CURRENT_USER Key \"%s\" Name \"%s\"\n"
            "No problem, will use the default settings "
            "(probably running Wasm for the first time)\n",
            sgRegistryKey, sgRegistryName);

        fUseDefaults = true;
        }
    else
        {
        if (20 != sscanf((char *)Buf,
                "%s %d %d %d %f %d %d %d %d %d %d %d %d %d"
                "%d %d %d %d %d %d",
                sPrevShapeFile, &iPrevImage, &iPrevLandmark,
                &fEqualize,  &EqAmount, &fConnectDots, &fShowNbrs,
                &fShowCurrentNbr, &fHideOriginalLandmarks, &fCrop, &iZoom,
                &fAutoNextImage, &fAutoNextPoint, &fJumpToLandmark,
                &xPos, &yPos, &xSize, &ySize, &xDlg, &yDlg) ||
            iPrevImage < 0 || iPrevImage > 100000 ||    // 100000 is arbitray
            iPrevLandmark < 0 || iPrevLandmark > 200 || // 200 is arbitray
            !fBool(fEqualize)              ||
            !fBool(fConnectDots)           ||
            !fBool(fShowNbrs)              ||
            !fBool(fShowCurrentNbr)        ||
            !fBool(fHideOriginalLandmarks) ||
            !fBool(fCrop)                  ||
            iZoom < 2 || iZoom > 7         ||
            !fBool(fAutoNextImage)         ||
            !fBool(fAutoNextPoint)         ||
            !fBool(fJumpToLandmark)        ||
            EqAmount < 0 || EqAmount > 1   ||
            xPos + xSize < 20              ||
            yPos + ySize < 20              ||
            xSize < 20                     ||
            ySize < 20)
            {
            lprintf("Can't get values from registry HKEY_CURRENT_USER "
                "Key \"%s\" Name \"%s\" %s %d %dx%d %dx%d\n"
                "No problem, will use the default settings "
                "(probably running %s for the first time)\n",
                sgRegistryKey, sgRegistryName,
                sPrevShapeFile, iPrevLandmark,
                xPos, yPos, xSize, ySize, sgProgramName);

            fUseDefaults = true;
            }
        }
    RegCloseKey(hKey);
    }
RECT RectWorkArea;
SystemParametersInfo(SPI_GETWORKAREA, 0, &RectWorkArea, 0);

if (fUseDefaults ||
        xSize > RectWorkArea.right - RectWorkArea.left + 100 ||
        ySize > RectWorkArea.bottom - RectWorkArea.top + 100 ||
        xPos < RectWorkArea.left - xSize + 40 ||
        yPos < RectWorkArea.top - ySize + 40 ||
        xDlg < RectWorkArea.left - ngDlgWidth + 40 ||
        yDlg < RectWorkArea.top - nDlgHeight + 40)
    {
    xSize = (RectWorkArea.right - RectWorkArea.left) / 2;
    ySize = iround((RectWorkArea.bottom - RectWorkArea.top) / 1.5);
    xPos = RectWorkArea.right - xSize;
    yPos = RectWorkArea.top;
    xDlg = xPos - ngDlgWidth;
    yDlg = yPos;
    iPrevImage = igImage;
    iPrevLandmark = igLandmark;
    fEqualize = false;
    EqAmount = 1;
    fConnectDots = false;
    fShowNbrs = false;
    fShowCurrentNbr = true;
    fHideOriginalLandmarks = false;
    fCrop = false;
    iZoom = 4;
    fAutoNextImage = false;
    fJumpToLandmark = false;
    fAutoNextPoint = false;
    }
if (igLandmark < 0) // not initialized by -l flag?
    igLandmark = iPrevLandmark;
igImage = iPrevImage;
fgEqualize = (fEqualize == 1);
if (fgSlowMachine)
    fgEqualize = false; // TODO equalization can cause crashes on Win98 machines?
gEqAmount = EqAmount;
fgConnectDots = (fConnectDots == 1);
fgShowNbrs = (fShowNbrs == 1);
fgShowCurrentNbr = (fShowCurrentNbr == 1);
fgHideOriginalLandmarks = (fHideOriginalLandmarks == 1);
fgCrop = (fCrop == 1);
igZoom = iZoom;
fgAutoNextImage = (fAutoNextImage == 1);
fgJumpToLandmark = false; // was fgJumpToLandmark = (fJumpToLandmark == 1);
fgAutoNextPoint = (fAutoNextPoint == 1);
*pxPos = xPos;
*pyPos = yPos;
*pxSize = xSize;
*pySize = ySize;
*pxDlg = xDlg;
*pyDlg = yDlg;
}

//-----------------------------------------------------------------------------
static void SaveStateToRegistry (HWND hMainWnd, HWND hDlgWnd)
{
RECT  rectMain, rectDlg;
HKEY  hKey;
DWORD dwDispo;
char  sRegValue[SLEN];

logprintf("Saving options to registry: HKEY_CURRENT_USER "
          "Key \"%s\" Name \"%s\"\n",
          sgRegistryKey, sgRegistryName);

GetWindowRect(hMainWnd, &rectMain);
GetWindowRect(hDlgWnd, &rectDlg);

int xDlg = rectDlg.left;
if (xDlg < 0)
    xDlg = 0;
int yDlg = rectDlg.left;
if (yDlg < 0)
    yDlg = 0;

sprintf(sRegValue,
    "%s %d %d %d %g %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
    sgShapeFile, igImage, igLandmark,
    fgEqualize,  gEqAmount, fgConnectDots, fgShowNbrs, fgShowCurrentNbr,
    fgHideOriginalLandmarks, fgCrop, igZoom,
    fgAutoNextImage, fgAutoNextPoint, fgJumpToLandmark,
    rectMain.left, rectMain.top,
    rectMain.right-rectMain.left, rectMain.bottom-rectMain.top,
    rectDlg.left, rectDlg.top);

// no point in checking return values in func calls below

RegCreateKeyEx(HKEY_CURRENT_USER, sgRegistryKey, 0, "", 0,
    KEY_ALL_ACCESS, NULL, &hKey, &dwDispo);
RegSetValueEx(hKey, sgRegistryName, 0, REG_SZ, (CONST BYTE *)sRegValue,
    strlen(sRegValue)+1);
RegCloseKey(hKey);
}

//-----------------------------------------------------------------------------
static double xMouseToScreen (double xMouse) // mouse to image coord
{
    return (xMouse * gImg.width) / ngMainWidth;
}

static double yMouseToScreen (double yMouse)
{
    return gImg.height -
           (yMouse * gImg.height) / (ngMainHeight + ngButtonHeight);
                                                  // TODO should be - not + ?
}

//-----------------------------------------------------------------------------
static double xScreenToShape (double xScreen)
{
    double x = igLeftCrop + xScreen -
                (gImg.width + igLeftCrop + igRightCrop)/2 + 0.5;

    return iround(x) - 1;   // TODO why is -1 needed?
}

static double yScreenToShape (double yScreen)
{
    double y = igBottomCrop -
                (gImg.height + igBottomCrop + igTopCrop)/2 + yScreen - 0.5;
    return iround(y);
}

//-----------------------------------------------------------------------------
static bool fPointMarked (int iImage, int iPoint)
{
return gShapesOrg[iImage](iPoint, VX) != gShapes[iImage](iPoint, VX) ||
       gShapesOrg[iImage](iPoint, VY) != gShapes[iImage](iPoint, VY);
}

//-----------------------------------------------------------------------------
static SHAPE DetVecToShape (const Mat &v)
{
DET_PARAMS DetParams;
VecToDetParams(DetParams, v);
return DetParamsToShape(DetParams);
}

//-----------------------------------------------------------------------------
static void DrawBigPoints (RgbImage &Img,           // io
                           const SHAPE &Shape,      // in
                           int iCurrentPoint)       // in
{
const unsigned Color = C_DRED;
const unsigned Red   = (Color >> 16) & 0xff;
const unsigned Green = (Color >> 8) & 0xff;
const unsigned Blue  = Color & 0xff;

SHAPE Shape1(Shape);
if (Shape.nrows() == 76)    // hack to display 76 point shapes nicely
    {
    Shape1 = End76ToMid76(Shape1);
    iCurrentPoint = iMid76ToEnd76[iCurrentPoint];
    }
for (int iPoint = 0; iPoint < (int)Shape.nrows(); iPoint++)
    {
    double x, y;
    GetPointCoords(x, y, iPoint, Shape1);
    if (iPoint != iCurrentPoint && fPointUsed(Shape1, iPoint))
        {
        DrawPoint(Img, x-1, y-1, Red, Green, Blue);
        DrawPoint(Img, x-1, y,   Red, Green, Blue);
        DrawPoint(Img, x-1, y+1, Red, Green, Blue);
        DrawPoint(Img, x, y-1, Red, Green, Blue);
        // DrawPoint(Img, x, y,   Red, Green, Blue);
        DrawPoint(Img, x, y+1, Red, Green, Blue);
        DrawPoint(Img, x+1, y-1, Red, Green, Blue);
        DrawPoint(Img, x+1, y,   Red, Green, Blue);
        DrawPoint(Img, x+1, y+1, Red, Green, Blue);
        }
    }
}

//-----------------------------------------------------------------------------
static void CreateCurrentDisplayImage (void)
{
extern void QuickEqualizeRgbImage(RgbImage &Img, double EqAmount);

double x, y;

ngImgWidth = gImg.width;
ngImgHeight = gImg.height;
if (!fgSlowMachine)                 // desaturate is slow
    DesaturateRgbImage(gImg);       // convert to gray
if (fgEqualize)
    QuickEqualizeRgbImage(gImg, gEqAmount);
if (igLandmark < 0 || igLandmark >= ngPoints)
    {
    igLandmark = 0;
    MsgBox("Out of range landmark number, forcing to 0");
    }
fgDetShape = false;
SHAPE Shape;
unsigned Attr;
DecomposeTag(gTags[igImage], &Attr);
if ((Attr & FA_ViolaJones) || (Attr & FA_Rowley))
    {
    // gShapes[igImage] is detector parameters, convert
    // those to a shape so we can display them

    Shape = DetVecToShape(gShapes[igImage]);
    fgDetShape = true;  // set flag so user cannot change landmarks with mouse
    }
else
    Shape = gShapes[igImage];
if (!fgHideOriginalLandmarks)
    {
    if (fgDetShape)
        {
        // face detector shape

        DrawShape(gImg, Shape, C_YELLOW,
            fgConnectDots, C_YELLOW, fgShowNbrs);

        if (fgConnectDots && xShapeExtent(Shape) > 150)
            {
            // draw shape again, displaced, so face box is more visible

            DrawShape(gImg, Shape+1, C_YELLOW, fgConnectDots, C_YELLOW);
            }
        }
    else
        {
        DrawShape(gImg, Shape, C_DRED,
            fgConnectDots, C_DRED, fgShowNbrs,
            fgShowCurrentNbr? igLandmark: -1);
#if 1
        DrawBigPoints(gImg, Shape, igLandmark);
#endif
        }

    // draw old mark

    GetPointCoords(x, y, igLandmark, gShapesOrg[igImage]);
    int Red = 0xff, Green = 0x20, Blue = 0x20; // ff 20 20 is bright red
    if (fPointMarked(igImage, igLandmark))
        {
        Red = 0xc0;     // c0 80 00 is orange
        Green = 0x80;
        Blue = 0;
        }
    DrawPoint(gImg, x, y, Red, Green, Blue);
    if (!fPointUsed(gShapesOrg[igImage], igLandmark))
        DrawCross(gImg, x, y, Red, Green, Blue);
    }
// draw new mark in cyan

for (int iPoint = 0; iPoint < ngPoints; iPoint++)
    if (fPointMarked(igImage, iPoint))
        {
        GetPointCoords(x, y, iPoint, gShapes[igImage]);
        int Red = 0x00, Green = 0xd0, Blue = 0xd0; // darkish cyan
        if (iPoint == igLandmark)
            {
            Red = 0x20;    // bright whitish cyan
            Green = 0xff;
            Blue = 0xff;
            }
        DrawPoint(gImg, x, y, Red, Green, Blue);
        if (!fPointUsed(gShapes[igImage], iPoint))
            DrawCross(gImg, x, y, Red, Green, Blue);
        }
// crop the image, depending on user setting of fgCrop

SHAPE *pShape = &gShapes[igImage];
if (!fgCrop) // don't crop
    igTopCrop = igBottomCrop = igRightCrop = igLeftCrop = 0;
else
    {
    static double xOld;
    static double yOld;

    // If in JumpToLandmark mode, then keep screen stable as much
    // as possible
    // TODO adjust this to be more useable

    if (fgJumpToLandmark &&
           xgMouse > 20 && xgMouse < ngMainWidth - 20 &&
           ygMouse > ngButtonHeight + 20 && ygMouse < ngMainHeight - 20)
       {
       x = xOld;
       y = yOld;
       }
    // If the mouse left button is down, then use the old x and y landmark
    // positions for cropping the image. This prevents movement of the image
    // as you mouse click to change a point's position.

    else if (fgLMouseDown)
       {
       x = xOld;
       y = yOld;
       }
    else
       {
       GetPointCoords(x, y, igLandmark, *pShape);
       xOld = x;
       yOld = y;
       }
    // put igLandmark in the center of the window
    int width = gImg.width, height = gImg.height;
    x = iround(x + width/2); // shape to window coords
    y = iround(y + height/2);
    igTopCrop    = 0;
    igBottomCrop = 0;
    igLeftCrop   = 0;
    igRightCrop  = 0;
    if (igZoom != 0)
        {
        igTopCrop    = iround(MAX(0, height - y - height/igZoom));
        igBottomCrop = iround(MAX(0, y - height/igZoom));
        igLeftCrop   = iround(MAX(0, x - width/igZoom));
        igRightCrop  = iround(MAX(0, width - x - width/igZoom));
        }
    }
// Note that CropRgbImage will alway crop images so their width is divisible
// by four, if necessary, regardless of the fgCrop setting.
// This allows us to use RgbPrintf (via DrawShape).

CropRgbImage(gImg,
    igTopCrop, igBottomCrop, igLeftCrop, igRightCrop,
    IM_WIDTH_DIVISIBLE_BY_4);
}

//-----------------------------------------------------------------------------
static char *sLoadCurrentImage (void)
{
const char *sErrMsg = NULL;
if (igPreloadedImage == igImage) // image is preloaded?
    gImg = gPreloadedImage;      // yes, use it
else
    {
    sErrMsg = sLoadImageGivenDirs(gImg,
                                  sGetBasenameFromTag(gTags[igImage].c_str()),
                                  sgImageDirs, sgShapeFile, NO_EXIT_ON_ERR);
    if (sErrMsg)
        {
        Err(sErrMsg);
        PostQuitMessage(0); // calling this here is not quite right but does the job
        }
    }
CreateCurrentDisplayImage();

return (char *)sErrMsg; // typecast discards const
}

//-----------------------------------------------------------------------------
// Preload the next image -- this makes moving to the next image faster on slow machines

static void PreloadImage (void)
{
int iNextImage = igImage + 1;
if (iNextImage >= ngImages)
    iNextImage = 0;
if (igPreloadedImage != iNextImage &&
    NULL == sLoadImageGivenDirs(gPreloadedImage,
                                sGetBasenameFromTag(gTags[iNextImage].c_str()),
                                sgImageDirs, sgShapeFile, NO_EXIT_ON_ERR))
        {
        igPreloadedImage = iNextImage; // succesfully loaded
        }
}

//-----------------------------------------------------------------------------
static void Redisplay (void)
{
DisplayButtons();
DisplayTitle();
sLoadCurrentImage();
InvalidateRect(hgMainWnd, NULL, false);     // force repaint of entire window
SetCursor(LoadCursor (NULL, IDC_CROSS));    // this is needed, not sure why

// Because the user may be holding the key down (auto-repeat, e.g. holding
// PageDn key to scroll through images) we need to force a screen update --
// else WM_PAINT gets queued behind WM_KEYDOWN and repaint only gets done
// after all keypresses stop.  Hence this call to WmPaint.

WmPaint(hgMainWnd);
}

//-----------------------------------------------------------------------------
static void StripQuotesAndBackQuote (char sStripped[], const char *sQuoted)
{
// remove quotes at each end of string, if any

strcpy(sStripped, &sQuoted[sQuoted[0] == '"'? 1:0]); // discard initial "
int iLen = strlen(sStripped)-1;
if (iLen > 0 && sStripped[iLen] == '"')
    sStripped[iLen] = 0;                             // discard final "

// replace backquote with space

for (int i = 0; sStripped[i]; i++)
    if (sStripped[i] == '`')
        sStripped[i] = ' ';
}

//-----------------------------------------------------------------------------
// return err msg if failure, NULL on success

static char *sProcessCmdLine (LPSTR pCmdLine)
{
char *sWhiteSpace = " \t";
char sStripped[10000];  // big string allows long regular expressions for -p flag

// A hack to deal with strtok: convert spaces inside quotes to backquote
// This allows us to place spaces inside strings in quotes (for regular
// expression and filenames)

int Len = strlen(pCmdLine);
for (int i = 0; i < Len; i++)
    if (pCmdLine[i] == '"')
        {
        i++;
        while (pCmdLine[i] && pCmdLine[i] != '"')
            {
            if (pCmdLine[i] == ' ')
                pCmdLine[i] = '`';
            i++;
            }
        }
char *sToken = strtok(pCmdLine, sWhiteSpace);
while (sToken != NULL)
    {
    if (sToken[0] == '-')
        {
        if (sToken[1] == 0)
            return sgUsage;
        switch (sToken[1])
            {
            case 'B':
                fgAllButtons = true;
                break;
            case 'F':
                fgUseRegEntries = false;
                break;
            case 'V':
                fgViewMode = true;
                fgLock = true;
                break;
            case 'l':
                sToken = strtok(NULL, sWhiteSpace);
                if (sToken == NULL)
                    return sgUsage;
                if(igStartLandmark != -1)
                    return "Cannot use both -l and -L flags.  Use -? for help.";
                igLandmark = -1;
                sscanf(sToken, "%d", &igLandmark);
                if (igLandmark < 0 || igLandmark > 200)
                    return "Illegal landmark number for -l flag.  Use -? for help.";
                break;
            case 'L':
                sToken = strtok(NULL, sWhiteSpace);
                if (sToken == NULL)
                    return sgUsage;
                if(igLandmark != -1)
                    return "Cannot use both -l and -L flags.  Use -? for help.";
                igStartLandmark = -1;
                sscanf(sToken, "%d", &igStartLandmark);
                if (igStartLandmark < 0 || igStartLandmark > 200)
                    return "Illegal landmark number for -L flag.  Use -? for help.";
                igLandmark = igStartLandmark;
                break;
            case 'o':
                sToken = strtok(NULL, sWhiteSpace);
                if (sToken == NULL)
                    return sgUsage;
                StripQuotesAndBackQuote(sStripped, sToken);
                if (!sStripped || sStripped[0] == 0 || sStripped[0] == '-')
                    return "Illegal filename for -o flag.  Use -? for help.";
                strcpy(sgNewShapeFile, sStripped);
                break;
            case 'p':
                sToken = strtok(NULL, sWhiteSpace);
                if (sToken == NULL)
                    return sgUsage;
                StripQuotesAndBackQuote(sStripped, sToken);
                if (!sStripped || sStripped[0] == 0 || sStripped[0] == '-')
                    return "Illegal string for -p flag.  Use -? for help.";
                strcpy(sgTagRegex, sStripped);
                break;
            case 'P':
                sToken = strtok(NULL, sWhiteSpace);
                if (sToken == NULL || 1 != sscanf(sToken, "%x", &gMask0))
                    return "-P needs two hex numbers.  Use -? for help.";
                sToken = strtok(NULL, sWhiteSpace);
                if (sToken == NULL || 1 != sscanf(sToken, "%x", &gMask1))
                    return "-P needs two hex numbers.  Use -? for help.";
                break;
            case 'd':
                sToken = strtok(NULL, sWhiteSpace);
                if (sToken == NULL)
                    return sgUsage;
                strcpy(sgFilterDir, sToken);
                Err("-d not yet implemented");
                break;
            case 't':
                fgTagMode = true;
                sToken = strtok(NULL, sWhiteSpace);
                if (sToken == NULL || 1 != sscanf(sToken, "%x", &gAttrMask))
                    return "-t needs a number.  Use -? for help.";
                break;
            case 'Q':
                fgQuiet = true;
                break;
            case 'S':
                fgSlowMachine = true;
                break;
            default:    // bad flag
                return sgUsage;
            }
        }
    else    // assume a string without '-' is the shape file name
        {
        StripQuotesAndBackQuote(sgShapeFile, sToken);
        strcpy(sgShapeFile, sToken);
        }
    sToken = strtok(NULL, sWhiteSpace);
    if (sgShapeFile[0] && sToken)
        return "Flags not allowed after the shape file, or extra tokens on the command line";
    }
if (!sgShapeFile[0])    // no shape file specified
    return "You must specify a shape file.  Use marki -? for help.\n"
           "\n"
           "Example: marki -o temp.shape -p \" i[^r]\" ../data/muct68.shape";

// make sure we can open the shape file

FILE *pFile = fopen(sgShapeFile, "r");
if (!pFile)
    {
    sprintf(sgBuf, "Can't open %s", sgShapeFile);
    return sgBuf;
    }
fclose(pFile);

if (sgNewShapeFile[0] == 0) // -o flag not used?
    strcpy(sgNewShapeFile, sgShapeFile);

lprintf("Input shape file %s\n", sgShapeFile);
lprintf("Output shape file %s\n", sgNewShapeFile);
if (gMask0)
    lprintf("Mask0 %x [%s] ", gMask0, sGetAtFaceString(gMask0));
if (gMask1)
    lprintf("Mask1 %x [%s] ", gMask1, sGetAtFaceString(gMask1, true));
if (fgTagMode)
    lprintf("Attr %x [%s] ", gAttrMask, sGetAtFaceString(gAttrMask));
if (sgTagRegex[0])
    lprintf("Regex \"%s\"\n", sgTagRegex);

return NULL;    // success
}

//-----------------------------------------------------------------------------
static void WmSize (HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
ngMainWidth = LOWORD(lParam);
ngMainHeight = HIWORD(lParam);
SendMessage(hgToolbar, TB_AUTOSIZE, 0, 0L);
}

//-----------------------------------------------------------------------------
// display a tooltip

static void WmNotify (HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
if (((LPNMHDR)lParam)->code == TTN_NEEDTEXT)
    {
    char s[2048];
    sprintf(s, "Save landmarks\n\nShape file:\n%s", sgNewShapeFile);
    ASSERT(gToolbarButtons[0].idCommand == IDM_Save);
    sgTooltips[0] = s; // [0] is IDM_Save
    LPTOOLTIPTEXT pToolTipText = (LPTOOLTIPTEXT)lParam;
    int           iButton = pToolTipText->hdr.idFrom - IDM_FirstInToolbar;
    LPSTR         pDest = pToolTipText->lpszText;
    strcpy(pDest, sgTooltips[iButton]);
    }
}

//-----------------------------------------------------------------------------
static void MsgBox (char *pArgs, ...)
{
va_list pArg;
char sMsg[1000];    // big enough for my strings

va_start(pArg, pArgs);
vsprintf(sMsg, pArgs, pArg);
va_end(pArg);

sMsg[0] = toupper(sMsg[0]);
lprintf("Mesage: %s\n", sMsg);
MessageBox(hgMainWnd, sMsg, sgProgramName, MB_OK);
}

//-----------------------------------------------------------------------------
// YesNoMsg returns true for YES

static BOOL fYesNoMsg (bool fDefaultYes, char *pArgs, ...)
{
va_list pArg;
char sMsg[1000];    // big enough for my strings

va_start(pArg, pArgs);
vsprintf(sMsg, pArgs, pArg);
va_end(pArg);

lprintf("YesNoMsg: %s", sMsg);

// use NULL for hwnd and not hgMainWnd because this may be
// called after hgMainWnd is closed
int iAnswer = MessageBox(NULL, sMsg, sgProgramName,
                MB_YESNO|MB_ICONQUESTION|(fDefaultYes? 0: MB_DEFBUTTON2));

if (sMsg[strlen(sMsg)-1] != '\n')
    lprintf(" ");

lprintf("%s\n", (iAnswer == IDYES? "YES": "NO"));

return IDYES == iAnswer;
}

//-----------------------------------------------------------------------------
// Functions for undo

typedef struct UNDO {
    int iImage;
    int iLandmark;
    double x;
    double y;
} UNDO;

static vector<UNDO> gUndo; // undo buffer
static vector<UNDO> gRedo; // redo buffer
static bool fgLastWasUndo;

static void SaveForUndo1 (vector<UNDO> &u)
{
    UNDO Undo;
    Undo.iImage = igImage;
    Undo.iLandmark = igLandmark;
    Undo.iLandmark = igLandmark;
    Undo.x = gShapes[igImage](igLandmark, VX);
    Undo.y = gShapes[igImage](igLandmark, VY);
    u.push_back(Undo);
}

static void SaveForUndo (void)
{
    SaveForUndo1(gUndo);
    gRedo.clear();
    fgLastWasUndo = false;
}

static void Undo (void)
{
    if (gUndo.size())
        {
        UNDO Undo(gUndo.back());
        gUndo.pop_back();
        SaveForUndo1(gRedo);
        igImage = Undo.iImage;
        igLandmark = Undo.iLandmark;
        UpdateLandmark(Undo.iImage, Undo.iLandmark, Undo.x, Undo.y, true);
        lprintf("Undo.x %g Undo.y %g\n", Undo.x, Undo.y);
        Redisplay();
        fgLastWasUndo = true;
        }
}

static void Redo (void)
{
    if (gRedo.size())
        {
        UNDO Undo(gRedo.back());
        gRedo.pop_back();
        SaveForUndo1(gUndo);
        igImage = Undo.iImage;
        igLandmark = Undo.iLandmark;
        UpdateLandmark(Undo.iImage, Undo.iLandmark, Undo.x, Undo.y, true);
        Redisplay();
        fgLastWasUndo = false;
        }
}

//-----------------------------------------------------------------------------
static void IdmNextImage (void)
{
if (++igImage > unsigned(ngImages) - 1)
    {
    igImage = 0;

    // make a sound when wrapped to first image, to let you know when you are done
    if (!fgQuiet) // -q command line flag
        PlaySound("C:/Windows/Media/notify.wav", NULL, SND_ASYNC|SND_FILENAME);

    #if 0
    MsgBox("Looped to first shape");
    #endif
    }
if (igStartLandmark != -1)
    igLandmark = igStartLandmark;
}

//-----------------------------------------------------------------------------
static void IdmPrevImage (void)
{
if (int(--igImage) < 0)
    igImage = ngImages-1;
if (igStartLandmark != -1)
    igLandmark = igStartLandmark;
}

//-----------------------------------------------------------------------------
static void SetLandmark (int iLandmark)
{
if (iLandmark != igLandmark)
    {
    igLandmark = iLandmark;
    if (igLandmark < 0)
        igLandmark = ngPoints - 1;
    else if (igLandmark >= ngPoints)
        igLandmark = 0;
    }
}

//-----------------------------------------------------------------------------
static void ZoomIn (void)
{
if (!fgCrop)
    fgCrop = true;
else
    {
    igZoom++;
    if (igZoom > 7)
        igZoom = 7;
    }
}

//-----------------------------------------------------------------------------
static void ZoomOut (void)
{
if (!fgCrop)
    fgCrop = true;
else
    {
    igZoom--;
    if (igZoom < 2)
        igZoom = 2;
    }
}

//-----------------------------------------------------------------------------
static void WriteShapeFile (const char sShapeFile[],   // all in
                            const vec_SHAPE &Shapes,
                            vec_string &Tags,
                            int nShapes, int nWantedPoints,
                            const char sImageDirs[])
{
FILE *pShapeFile = Fopen(sShapeFile, "w");
time_t ltime; time(&ltime);
Fprintf(pShapeFile, "ss %s %s\n", sShapeFile, ctime(&ltime));
Fprintf(pShapeFile, "Directories %s\n\n", sImageDirs);
ASSERT(Shapes[0].nrows() > 0); // sanity check to track down a possible bug
for (int iShape = 0; iShape < nShapes; iShape++)
    {
    if (Tags[iShape][0])
        {
        unsigned Attr; char sImage[SLEN];
        DecomposeTag(Tags[iShape], &Attr, sImage);
        Fprintf(pShapeFile, "\"%4.4x %s\"\n", Attr, sImage);
        }
    Shapes[iShape].write(sShapeFile, pShapeFile, "%.1f");
    }
fclose(pShapeFile);
lprintf("\n");
}

//-----------------------------------------------------------------------------
static void SaveState (void)
{
fgMarkedFilesUpToDate = true;
lprintf("Writing \"%s\" ", sgNewShapeFile);
lprintf("(%d landmarks per shape) ",  ngPoints);
WriteShapeFile(sgNewShapeFile, gShapes, gTags, ngImages, ngPoints, sgImageDirs);
}

//-----------------------------------------------------------------------------
static void UpdateDlgWnd (void)
{
char s[SLEN];

sprintf(s, "%d", igImage);
SetDlgItemText(hgDlgWnd, IDC_IMAGE_NBR, s);

sprintf(s, "of %d", ngImages);
SetDlgItemText(hgDlgWnd, IDC_NBR_IMAGES, s);

sprintf(s, "%d", igLandmark);
SetDlgItemText(hgDlgWnd, IDC_LANDMARK, s);

sprintf(s, "of %d", ngPoints);
SetDlgItemText(hgDlgWnd, IDC_NBR_LANDMARKS, s);

char *sMarked = "";
//TODO it would be nice to get rid of this loop
for (int iPoint = 0; iPoint < ngPoints; iPoint++)
    if (fPointMarked(igImage, iPoint))
        {
        sMarked = "Position changed ";
        break;
        }
char *sAttr = "";
if (gTags.size())     // initialized?
    {
    unsigned Attr;
    DecomposeTag(gTags[igImage], &Attr);
    sAttr = sGetAtFaceString(Attr);
    }
sprintf(s, "%s%s%s", sMarked, (sMarked[0]? " ":""), sAttr);
SetDlgItemText(hgDlgWnd, IDC_MARKED, s);

SetDlgItemText(hgDlgWnd, IDC_SAVED,
    (fgMarkedFilesUpToDate? "": "Changes not saved"));

if (gTags.size() > igImage)
    {
    sprintf(s, "%s    %dx%d   ", gTags[igImage].c_str(), ngImgWidth, ngImgHeight);
    SetDlgItemText(hgDlgWnd, IDC_FILE_NAME, s);
    }
}

//-----------------------------------------------------------------------------
BOOL CALLBACK DlgProc (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
char s[SLEN];

switch (msg)
    {
    case WM_INITDIALOG:
        {
        // Move the dialog window to the right place.  It's actually a
        // top level window so we need to position wrt the screen

        RECT RectWorkArea;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &RectWorkArea, 0);
        MoveWindow(hDlg, xgDlg, ygDlg, ngDlgWidth, nDlgHeight+30, true);
        SendDlgItemMessage(hDlg, IDC_IMAGE_NBR, EM_LIMITTEXT, SLEN, 0);
        SendDlgItemMessage(hDlg, IDC_LANDMARK,  EM_LIMITTEXT, SLEN, 0);
        char s[SLEN]; sprintf(s, "%s %s", sgProgramName, MARKI_VERSION);
        SetWindowText(hDlg, s);
        UpdateDlgWnd();
        return false;
        }
    case WM_COMMAND:
        // Hack -- make hitting return in an edit box the same as clicking on OK.
        // This only works if ES_MULTILINE is used in marki.rc
        // TODO clean this up, what is the proper way to do this?

        #define isReturn(wParam) (((wParam >> 16) & 0xffff) == EN_MAXTEXT)

        // TODO why doesn't hitting tab move you to the next control?

        if (wParam == IDOK || isReturn(wParam))
            {
            GetDlgItemText(hDlg, IDC_LANDMARK, s, SLEN);
            int iInput = -1;
            if (1 != sscanf(s, "%d", &iInput) || iInput < 0 || iInput >= ngPoints)
                {
                MsgBox("bad landmark number \"%s\", allowed range is 0 to %d",
                    s, ngPoints-1);
                sprintf(s, "%d", igLandmark);
                SetDlgItemText(hDlg, IDC_LANDMARK, s);
                }
            else
                {
                if (iInput != igLandmark)
                    {
                    SaveForUndo();
                    SetLandmark(iInput);
                    }
                GetDlgItemText(hDlg, IDC_IMAGE_NBR, s, SLEN);
                iInput = -1;
                if (1 != sscanf(s, "%d", &iInput) || iInput < 0 || iInput >= ngImages)
                    {
                    MsgBox("bad image number \"%s\", allowed range is 0 to %d",
                        s, ngImages-1);
                    sprintf(s, "%d", igImage);
                    SetDlgItemText(hDlg, IDC_IMAGE_NBR, s);
                    }
                else if (iInput != int(igImage))
                    {
                    SaveForUndo();
                    igImage = iInput;
                    }
                }
            Redisplay();
            SetFocus(hgMainWnd); // set focus back to main window
            return true;
            }
    }
return false;
}

//-----------------------------------------------------------------------------
static void DisplayButton (int idm, int flag)
{
SendMessage(hgToolbar, TB_CHECKBUTTON, idm, (LPARAM)MAKELONG(flag, 0));
}

//-----------------------------------------------------------------------------
// tell the buttons to display themselves as depressed or not

static void DisplayButtons (void)
{
DisplayButton(IDM_Lock,              fgLock);
DisplayButton(IDM_Equalize,          fgEqualize);
DisplayButton(IDM_ConnectDots,       fgConnectDots);
DisplayButton(IDM_ShowNbrs,          fgShowNbrs);
DisplayButton(IDM_ShowCurrentNbr,    fgShowCurrentNbr);
DisplayButton(IDM_HideLandmarks,     fgHideOriginalLandmarks);
DisplayButton(IDM_Crop,              fgCrop);
DisplayButton(IDM_AutoNextImage,     fgAutoNextImage);
DisplayButton(IDM_JumpToLandmark,    fgJumpToLandmark);
DisplayButton(IDM_AutoNextPoint,     fgAutoNextPoint);
}

//-----------------------------------------------------------------------------
void WmCommand (HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
const int nRepeats = (fgControlKeyDown? 10: 1);
int iRepeat;
sgSavedImage[0] = 0;

switch (wParam)
    {
    case IDM_Save:
        SaveState();
        MsgBox("Saved to %s", sgNewShapeFile);
        fgLock = true;
        break;

    case IDM_Lock:
        if(!fgViewMode)
            fgLock ^= 1;
        else if (fgLock)
            MsgBox("Cannot unlock in \"View mode\"");
        break;

    case IDM_Undo:
        Undo();
        break;

    case IDM_Redo:
        Redo();
        break;

    case IDM_WriteImage:
        {
        char sPath[SLEN];
        GetSaveBmpFilename(sPath, hWnd);             // get sPath from the user
        if (sPath[0])                                // valid path?
            {
            WriteBmp(gImg, sgSavedImage, VERBOSE);
            MsgBox("Wrote %s", sgSavedImage);
            }
        break;
        }

    case IDM_Equalize:
        if (fgSlowMachine) // TODO equalization can cause crashes on Win98 machines?
            MsgBox("Equalization not available (-S flag was used)");
        else
            fgEqualize ^= 1;
        break;

    case IDM_ConnectDots:
        fgHideOriginalLandmarks = 0;
        fgConnectDots ^= 1;
        break;

    case IDM_ShowNbrs:
        fgHideOriginalLandmarks = 0;
        fgShowNbrs ^= 1;
        break;

    case IDM_ShowCurrentNbr:
        fgHideOriginalLandmarks = 0;
        fgShowCurrentNbr ^= 1;
        break;

    case IDM_HideLandmarks:
        fgHideOriginalLandmarks ^= 1;
        break;

    case IDM_Crop:
        fgCrop ^= 1;
        break;

    case IDM_ZoomOut:
        ZoomOut();
        break;

    case IDM_ZoomIn:
        ZoomIn();
        break;

    case IDM_PrevImage:
        SaveForUndo();
        for (iRepeat = 0; iRepeat < nRepeats; iRepeat++)
            IdmPrevImage();
        break;

    case IDM_NextImage:
        SaveForUndo();
        for (iRepeat = 0; iRepeat < nRepeats; iRepeat++)
            IdmNextImage();
        break;

    case IDM_AutoNextImage:
        fgAutoNextImage ^= 1;
        if (fgAutoNextImage)
            fgAutoNextPoint = false;
        break;

    case IDM_JumpToLandmark:
        fgJumpToLandmark ^= 1;
        fgAutoNextPoint = false;
        break;

    case IDM_AutoNextPoint:
        fgAutoNextPoint ^= 1;
        fgJumpToLandmark = false;
        if (fgAutoNextPoint)
            fgAutoNextImage = false;
        break;

    case IDM_PrevPoint:
        if (fgDetShape)
            MsgBox("Can't change landmarks in a face detector shape");
        else
            {
            fgJumpToLandmark = false;
            SaveForUndo();
            for (iRepeat = 0; iRepeat < nRepeats; iRepeat++)
                SetLandmark(igLandmark - 1);
            }
        break;

    case IDM_NextPoint:
        if (fgDetShape)
            MsgBox("Can't change landmarks in a face detector shape");
        else
            {
            fgJumpToLandmark = false;
            SaveForUndo();
            for (iRepeat = 0; iRepeat < nRepeats; iRepeat++)
                SetLandmark(igLandmark + 1);
            }
        break;

    case IDM_Help:
        {
        char s[10000];
        sprintf(s, "%s\n\n%s %s\nwww.milbo.org\n", sgHelp, sgProgramName, MARKI_VERSION);
        MessageBox(hgMainWnd, s, sgProgramName, MB_OK);
        }
        break;

    case IDM_Blank:
        break;

    default:
        Err("WmCommand bad param %u", wParam);
        break;
    }
if (wParam != IDM_Blank)
    Redisplay();

// Hack: clear global error flag that was possibly set in a previous call to Err
// (If there was a call to Err, the user would have seen a popup error message.)
// This is needed because in a Windows environment we don't exit on Err (as we
// do in a command line environment).  Various code in stasm checks fgErr and
// aborts if it is set.  So we don't want previous errors to cause problems
// now i.e. start with a clean slate after any of the above IDM commands.

fgErr = false;
}

//-----------------------------------------------------------------------------
static void CheckSaveState (void)
{
if (!fgMarkedFilesUpToDate &&
        fYesNoMsg(true,
                  "Save new landmark positions?\n\nWill save to:\n%s\n\n",
                  sgNewShapeFile))
    {
    SaveState();
    }
}

//-----------------------------------------------------------------------------
// display the image -- magnify it so it fills the window

static void WmPaint (HWND hWnd)
{
HDC         hdc;
PAINTSTRUCT ps;

BITMAPINFO BmInfo;
memset(&BmInfo.bmiHeader, 0, sizeof(BmInfo.bmiHeader));
BmInfo.bmiHeader.biSize     = 40;
BmInfo.bmiHeader.biWidth    = gImg.width;
BmInfo.bmiHeader.biHeight   = gImg.height;
BmInfo.bmiHeader.biPlanes   = 1;
BmInfo.bmiHeader.biBitCount = 24;

hdc = BeginPaint(hWnd, &ps);

SetStretchBltMode(hdc, COLORONCOLOR);

StretchDIBits(hdc,
    0, ngButtonHeight, ngMainWidth, ngMainHeight - ngButtonHeight,
    0, 0,                       // xSrcLowerLeft, ySrcLowerLeft
    gImg.width, gImg.height,    // SrcWidth, SrcHeight
    gImg.buf,                   // lpBits
    (LPBITMAPINFO)&BmInfo,      // lpBitsInfo
    DIB_RGB_COLORS,             // wUsage
    SRCCOPY);                   // raser operation code

EndPaint(hWnd, &ps);
UpdateDlgWnd();
PreloadImage();
}

//-----------------------------------------------------------------------------
static void WmKeydown (HWND hWnd, WPARAM wParam)
{
bool fRedisplay = true;
sgSavedImage[0] = 0;
const int nRepeats = (fgControlKeyDown? 10: 1);
int iRepeat;

switch (wParam)
    {
    case VK_CONTROL:           // control
        fgControlKeyDown = true;
        fRedisplay = false;
        break;
    case ' ':                   // space
    case VK_NEXT:               // page down
        SaveForUndo();
        for (iRepeat = 0; iRepeat < nRepeats; iRepeat++)
            IdmNextImage();
        break;
    case VK_PRIOR:              // page up
        SaveForUndo();
        for (iRepeat = 0; iRepeat < nRepeats; iRepeat++)
            IdmPrevImage();
        break;
    case VK_DOWN:               // arrow down
    case VK_RIGHT:              // arrow right
        if (!fgAllButtons)
            MsgBox("Can't change landmark number with arrow keys\n"
                "(use -B flag to allow this or use the dialog window))");
        else if (fgDetShape)
            MsgBox("Can't change landmarks in a face detector shape");
        else
            {
            SaveForUndo();
            for (iRepeat = 0; iRepeat < nRepeats; iRepeat++)
                SetLandmark(igLandmark + 1);
            }
        break;
    case VK_UP:                 // arrow up
    case VK_LEFT:               // arrow left
        if (!fgAllButtons)
            MsgBox("Can't change landmark number with arrow keys\n"
                "(use -B flag to allow this or use the dialog window))");
        else if (fgDetShape)
            MsgBox("Can't change landmarks in a face detector shape");
        else
            {
            SaveForUndo();
            for (iRepeat = 0; iRepeat < nRepeats; iRepeat++)
                SetLandmark(igLandmark - 1);
            }
        break;
    case VK_HOME:               // goto first image
        SaveForUndo();
        igImage = 0;
        if (igStartLandmark != -1)
            igLandmark = igStartLandmark;
        break;
    case VK_END:                // goto last image
        SaveForUndo();
        igImage = ngImages-1;
        if (igStartLandmark != -1)
            igLandmark = igStartLandmark;
        break;
    default:                    // unrecognized key, ignore it
        fRedisplay = false;
    }
if (fRedisplay)
    Redisplay();
}

//-----------------------------------------------------------------------------
static void MarkCurrentPointAsUnused (void)
{
SaveForUndo();
}

//-----------------------------------------------------------------------------
static void WmChar (HWND hWnd, WPARAM wParam)
{
bool fRedisplay = true;
switch (wParam)
    {
    case 'C':                   // crop
    case 'c':
        fgCrop ^= 1;
        break;
    case '-':
        ZoomOut();
        break;
    case '+':
        ZoomIn();
        break;
    case 'E':                   // equalize
    case 'e':
        if (fgSlowMachine)
            MsgBox("Equalization not available (-S flag was used)");
        else
            fgEqualize ^= 1;
        break;
    case '[':
        if (fgSlowMachine)
            MsgBox("Equalization not available (-S flag was used)");
        else
            {
            fgEqualize = true;
            gEqAmount -= 0.2;
            if (gEqAmount < 0.2)
                gEqAmount = 0.2;
            }
        break;
    case ']':
        if (fgSlowMachine)
            MsgBox("Equalization not available (-S flag was used)");
        else
            {
            fgEqualize = true;
            gEqAmount += 0.2;
            if (gEqAmount > 1)
               gEqAmount = 1;
            }
        break;
    case 'D':                   // connect the dots
    case 'd':
        fgConnectDots ^= 1;
        break;
    case 'H':
    case 'h':
        fgHideOriginalLandmarks ^= 1;
        break;
    case 'N':
    case 'n':
        fgShowCurrentNbr ^= 1;
        break;
    case 'X':                   // mark point as unused
    case 'x':
        if (fPointUsed(gShapes[igImage], igLandmark))
            {
            SaveForUndo();
            UpdateLandmark(igImage, igLandmark, 0, 0, false);
            if (fgAutoNextImage)
                IdmNextImage();
            else if (fgAutoNextPoint)
                SetLandmark(igLandmark+1);
            }
        else
            MsgBox("Point %d is already unused", igLandmark);
        break;
    case 0x0d:                  // allow user to put blank lines in command window
        lprintf("\n");
        break;
    case 0x1a:                  // controlZ
        if (fgLastWasUndo)
            Redo();
        else
            Undo();
        break;
    default:                    // unrecognized character, ignore it
        fRedisplay = false;
    }
if (fRedisplay)
    Redisplay();
}

//-----------------------------------------------------------------------------
static void UpdateLandmark (int iImage, int iLandmark,
                            double x, double y, bool fIsUndo)
{
igLandmark = iLandmark;
igImage = iImage;

if (!fIsUndo ||
        gShapes[igImage](igLandmark, VX) != x || // position of landmark changed?
        gShapes[igImage](igLandmark, VY) != y)
    {
    lprintf("Mark \"%s\" Landmark %d Position %6.1f %6.1f (original %6.1f %6.1f)\n",
        gTags[igImage].c_str(),
        igLandmark, x, y,
        gShapesOrg[igImage](igLandmark, VX), gShapesOrg[igImage](igLandmark, VY));

    gShapes[igImage](igLandmark, VX) = x;
    gShapes[igImage](igLandmark, VY) = y;

    fgMarkedFilesUpToDate = false;
    }
}

//-----------------------------------------------------------------------------
static int iGetClosestLandmark (int xMouse, int yMouse)
{
double x = xScreenToShape(xMouseToScreen(xMouse));
double y = yScreenToShape(yMouseToScreen(yMouse));
double Min = 1e10;
int iBest = -1;
for (int iPoint = 0; iPoint < ngPoints; iPoint++)
    {
    double x1, y1;
    GetPointCoords(x1, y1, iPoint, gShapes[igImage]);
    double Dist = (x - x1) * (x - x1) + (y - y1) * (y - y1);
    if (Dist < Min)
        {
        Min = Dist;
        iBest = iPoint;
        }
    }
return iBest;
}

//-----------------------------------------------------------------------------
static void DisplayTitle (double xMouse, double yMouse)
{
    static double xMouseOld, yMouseOld;
    if (xMouse <= -10000) // parameter not used?
        {                 // if so, save mouse positions
        xMouse = xMouseOld;
        yMouse = yMouseOld;
        }
    xMouseOld = xMouse;
    yMouseOld = yMouse;
    double xScreen = xMouseToScreen(xMouse);    // mouse to image coords
    double yScreen = yMouseToScreen(yMouse);
    double x = xScreenToShape(xScreen);         // image to shape coords
    double y = yScreenToShape(yScreen);
    char sZoom[SLEN]; sZoom[0] = 0;
    if (fgCrop)
        sprintf(sZoom, "zoom %d    ", igZoom);
    char sEq[SLEN]; sEq[0] = 0;
    if (fgEqualize)
        sprintf(sEq, "eq %g", gEqAmount);
    char *sView = "";
    if (fgViewMode)
        sView = "VIEW ";
    else if (fgTagMode)
        sView = "TAG |";
    char s[SLEN];
    sprintf(s, "%s %s     %6.0f %6.0f    %s        %s%s   %s",
        sView, (gTags.size() > igImage)? gTags[igImage].c_str(): "",
        x, y, sgSavedImage, sZoom, sEq, sgShapeFile);
    SetWindowText(hgMainWnd, s);
}

//-----------------------------------------------------------------------------
static bool fShapeFileIsWriteable (void)
{
static int iCanWrite = -1;
if (iCanWrite == -1) // first time?
    {
    // make sure we can open the new shape file (so can save changes later)
    iCanWrite = 0;
    FILE *pFile = fopen(sgNewShapeFile, "a");
    if (pFile != NULL)
        {
        iCanWrite = 1;
        fclose(pFile);
        }
    }
return iCanWrite == 1;
}

//-----------------------------------------------------------------------------
static int Signed (unsigned x)
{
if (x > 0x7fff)
    x -= 0x10000;
return x;
}

//-----------------------------------------------------------------------------
static void Wm_LButtonDown1 (LPARAM lParam, bool fSaveForUndo)
{
if(fgViewMode)
    MsgBox("Landmark positions locked in \"View mode\"");
else if (fgLock)
    MsgBox("Landmark positions locked --- click on the Unlock button to unlock them");
else if (!fShapeFileIsWriteable())
    MsgBox("Mouse click ignored:\nthe shape file %s is not writeable",
        sgNewShapeFile);
else if (fgDetShape)  // current shape is not face detector shape?
    MsgBox("Can't change a face detector shape");
else
    {
    fgLMouseDown = true;
    if (fSaveForUndo)
        SaveForUndo();
    if (fgTagMode)
        TagCurrentShape(false);
    else
        {
        xgMouse = Signed(LOWORD(lParam));
        ygMouse = Signed(HIWORD(lParam));
        double xScreen = xMouseToScreen(xgMouse);    // mouse to image coords
        double yScreen = yMouseToScreen(ygMouse);
        double x = xScreenToShape(xScreen);         // image to shape coords
        double y = yScreenToShape(yScreen);
        // make sure coords aren't 0,0 because that means "unused landmark"
        if (x == 0 && y == 0)
            x = 0.1;
        // capture the mouse even when mouse is off the window, Petzold Win95 p322
        SetCapture(hgMainWnd);
        UpdateLandmark(igImage, igLandmark, x, y, false);
        }
    if(!(fgSlowMachine && fgAutoNextImage))
        Redisplay();
    }
}

static void Wm_LButtonDown (LPARAM lParam)
{
Wm_LButtonDown1(lParam, true);
}

//-----------------------------------------------------------------------------
static void Wm_LButtonUp (LPARAM lParam)
{
fgLMouseDown = false;
ReleaseCapture();
if (fgAutoNextImage)
    IdmNextImage();
else if (fgAutoNextPoint)
    SetLandmark(igLandmark+1);
if (fgAutoNextImage || fgAutoNextPoint)
    Redisplay();
}

//-----------------------------------------------------------------------------
static void WmMouseMove (LPARAM lParam)
{
static LPARAM lParamOld;

// Windows feeds us repeated WM_MOUSE messages even
// when the mouse hasn't moved.  Hence the if below.

if (lParam != lParamOld)
    {
    xgMouse = Signed(LOWORD(lParam));
    ygMouse = Signed(HIWORD(lParam));
    lParamOld = lParam;
    if (fgJumpToLandmark && !fgLMouseDown) // prevent landmarking switching while dragging
        {
        igLandmark = iGetClosestLandmark(xgMouse, ygMouse);
        Redisplay();
        }
    DisplayTitle(xgMouse, ygMouse);
    if (fgLMouseDown)
        Wm_LButtonDown1(lParam, false);
    }
SetCursor(LoadCursor (NULL, IDC_CROSS));
}

//-----------------------------------------------------------------------------
static void TagCurrentShape (bool fTag)
{
unsigned Attr; char sImage[SLEN];
DecomposeTag(gTags[igImage], &Attr, sImage);
unsigned OldAttr = Attr;
if (fTag)
    Attr |= gAttrMask;
else
    Attr &= ~gAttrMask;
char sNewTag[SLEN];
sprintf(sNewTag, "%4.4x %s", Attr, sImage);
gTags[igImage] = sNewTag;
if (Attr != OldAttr)
    {
    fgMarkedFilesUpToDate = false;
    // lprintf("New tag %s %s\n", sNewTag, ((Attr != OldAttr)? "Changed": "No change"));
    lprintf("New tag %s\n", sNewTag);
    }
}

//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc (HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
switch (iMsg)
    {
    case WM_CREATE:
        if (NULL == (hgDlgWnd = CreateDialog(hgAppInstance, "Dlg", hWnd, DlgProc)))
            Err("CreateDialog failed");
        return 0;

    case WM_PAINT:
        WmPaint(hWnd);
        return 0;

    case WM_SIZE:
        WmSize(hWnd, iMsg, wParam, lParam);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_CLOSE:
        CheckSaveState();
        SaveStateToRegistry(hgMainWnd, hgDlgWnd);
        break;  // call default window close handler

    case WM_COMMAND:    // toolbar buttons
        WmCommand(hWnd, iMsg, wParam, lParam);
        return 0;

    case WM_NOTIFY:     // display a tooltip
        WmNotify(hWnd, iMsg, wParam, lParam);
        return 0;

    case WM_LBUTTONDOWN:    // mouse left click, set landmark position
        Wm_LButtonDown(lParam);
        break;

    case WM_LBUTTONUP:     // mouse left click up
        Wm_LButtonUp(lParam);
        break;

    case WM_RBUTTONDOWN:    // mouse right click, toggle equalize
        if (fgTagMode)
            {
            if(fgViewMode)
                MsgBox("Landmark positions locked in \"View mode\"");
            else if (fgLock)
                MsgBox("Landmark positions locked --- click on the Unlock button to unlock them");
            else if (!fShapeFileIsWriteable())
                MsgBox("Mouse click ignored:\nthe shape file %s is not writeable",
                    sgNewShapeFile);
            else if (fgDetShape)  // current shape is not face detector shape?
                MsgBox("Can't change a face detector shape");
            else
                TagCurrentShape(true);
            }
        else if (fgSlowMachine)
            MsgBox("Equalization not available (-S flag was used)");
        else
            fgEqualize ^= 1;
        fgJumpToLandmark = false; // TODO inform user?
        Redisplay();
        break;

    case WM_RBUTTONUP:
        if (fgTagMode && fgAutoNextImage)
            {
            IdmNextImage();
            Redisplay();
            }
        break;

     case WM_MBUTTONDOWN:   // mouse middle click
        Redisplay();        // this is needed to fix cursor, not sure why
        break;

    case WM_MOUSEMOVE:
        WmMouseMove(lParam);
        break;

    case WM_MOUSEWHEEL:
        if (short(HIWORD(wParam)) > 0)
            SendMessage(hWnd, WM_KEYDOWN, VK_NEXT, 0);  // PageDn
        else
            SendMessage(hWnd, WM_KEYDOWN, VK_PRIOR, 0); // PageUp
        return 0;

    case WM_KEYDOWN:        // user hit a key
        WmKeydown(hWnd, wParam);
        return 0;

    case WM_KEYUP:          // user released key
        if (wParam == VK_CONTROL)
            fgControlKeyDown = false;
        return 0;

    case WM_CHAR:
        WmChar(hWnd, wParam);
        return 0;
    }
return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
static char *sCreateToolbar (HWND hWnd) // return err msg, or null if no error
{
hgToolbarBmp = LoadBitmap(hgAppInstance, MAKEINTRESOURCE(IDR_MARKI_BUTTONS));
if (!hgToolbarBmp)
    return "CreateToolbar failed, no BMP";

TBBUTTON *pButtons = fgAllButtons? gToolbarButtonsAll: gToolbarButtons;

int nButtons;
for (nButtons = 0; nButtons < 100; nButtons++)       // calculate nButtons
    if (pButtons[nButtons].iBitmap < 0)
        break;

hgToolbar = CreateToolbarEx(
        hWnd,                   // parent window
        WS_VISIBLE|TBSTYLE_FLAT|TBSTYLE_TOOLTIPS,
        1,                      // child ID of the this toolbar child window
        1,                      // nBitmaps
        NULL,                   // hBmInst
        (UINT)hgToolbarBmp,     // wBMID
        pButtons,               // lpButtons
        nButtons,               // iNumButtons (must include Separators)
        0, 0, 0, 0,             // position etc.
        sizeof(TBBUTTON));

if (!hgToolbar)
    return "CreateTooobar failed";

return NULL;
}

//-----------------------------------------------------------------------------
char *sInit (HINSTANCE hInstance, HINSTANCE hPrevInstance,
             LPSTR pCmdLine, int iCmdShow)
{
WNDCLASSEX  wndclass;
hgAppInstance   = hInstance;

if (!fgViewMode) // in view mode don't overwrite any other possible incarnation of Marki
    if (NULL == (pgLogFile = fopen(sgMarkiLog, "wt"))) // pgLogFile is defined in util.cpp
        return "Can't open log file";

gStartTime = clock();

if (pgLogFile)
    fprintf(pgLogFile, "%s %s\n", sgProgramName, MARKI_VERSION);

wndclass.cbSize        = sizeof(wndclass);
wndclass.style         = CS_HREDRAW | CS_VREDRAW;
wndclass.lpfnWndProc   = WndProc;
wndclass.cbClsExtra    = 0;
wndclass.cbWndExtra    = 0;
wndclass.hInstance     = hInstance;
wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
wndclass.lpszMenuName  = NULL;
wndclass.lpszClassName = sgClass;
wndclass.hIcon         = LoadIcon(hInstance, "Marki");
wndclass.hIconSm       = LoadIcon(hInstance, "Marki");

if (!RegisterClassEx(&wndclass))
    return "RegisterClass failed";

// Create the same window layout as last time by looking at registry.
// If nothing in registry, then GetStateFromRegistry returns
// biggest possible main wind.
// This also sets a whole bunch of globals like igImage and igLandmark.

int xPos, yPos, xSize, ySize;
GetStateFromRegistry(&xPos, &yPos, &xSize, &ySize, &xgDlg, &ygDlg);
if (int(igImage) < 0)
    igImage = 0;
if (igLandmark < 0)
    igLandmark = 0;
// no auto-next-landmark if user can't change current landmark number within Marki
if (!fgAllButtons)
    {
    fgAutoNextPoint = false;
    fgJumpToLandmark = false;
    }
hgMainWnd = CreateWindow(sgProgramName,
        sgProgramName,          // window caption
        WS_OVERLAPPEDWINDOW,    // window style
        xPos,                   // x position
        yPos,                   // y position
        xSize,                  // x size
        ySize,                  // y size
        NULL,                   // parent window handle
        NULL,                   // window menu handle
        hInstance,              // program instance handle
        NULL);                  // creation parameters

if (!hgMainWnd)
    return "CreateWindowEx failed";

if (char *sErrMsg = sCreateToolbar(hgMainWnd))
    return sErrMsg;

return NULL;                // success
}

//-----------------------------------------------------------------------------
// This should always be called before exiting the program

static int fShutdown (char *pArgs, ...)  // always returns 0
{
va_list pArg;
char sMsg[1000];    // big enough for my strings
if (!pgLogFile)
    pgLogFile = stdout;
sMsg[0] = 0;
if (pArgs)
    {
    va_start(pArg, pArgs);
    vsprintf(sMsg, pArgs, pArg);
    va_end(pArg);

    Err(sMsg);
    }
if (sMsg && sMsg[0])
    {
    if (pgLogFile)
        fprintf(pgLogFile, "MsgBox %s\n", sMsg);
    CheckSaveState();   // give user chance to save if we have a fatal error
    }
clock_t ElapsedTime = clock() - gStartTime;

// print time if we have been in this prog for more than 10 secs
// (so we don't print the time if an error message on bootup)
// TODO why does this message get printed twice? it doesn't if I use printf, not lprintf
if (ElapsedTime/CLOCKS_PER_SEC > 10)
    lprintf("%s elapsed\n", sFormatTime(double(ElapsedTime)));

if (hgAppInstance)
    {
    UnregisterClass(sgProgramName, hgAppInstance);
    hgAppInstance = NULL;
    }
if (hgToolbar)
    {
    DestroyWindow(hgToolbar);
    hgToolbar = NULL;
    }
if (hgToolbarBmp)
    {
    DeleteObject((HGDIOBJ)hgToolbarBmp);
    hgToolbarBmp = NULL;
    }
return 0;
}

//-----------------------------------------------------------------------------
static void ReportNoImages (void)
{
if (sgTagRegex[0] || gMask0 != 0 || gMask1 != 0)
    {
    char s[SLEN]; strcpy(s, sGetAtFaceString(gMask0));

    Err("no shapes in %s match \"%s\" Mask0 %x [%s] Mask1 %x [%s] ",
        sgShapeFile, sgTagRegex,
        gMask0, s, gMask1, sGetAtFaceString(gMask1, true));
    }
else
    Err("no shapes");
}

//-----------------------------------------------------------------------------
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR pCmdLine, int iCmdShow)
{
MSG msg;
char *sErrMsg;

if (sErrMsg = sProcessCmdLine(pCmdLine))
    {
    Err(sErrMsg);
    return fShutdown(NULL);
    }
if (!fgViewMode)
   {
   // shutdown if this program is running already (else the log files get mixed up)
   CreateMutex(NULL, true, sgProgramName);
   if (GetLastError() == ERROR_ALREADY_EXISTS)
       return fShutdown("Marki is running already\n(Use -V flag to run Marki twice)");
   }
if (sErrMsg = sInit(hInstance, hPrevInstance, pCmdLine, iCmdShow))
    return fShutdown(sErrMsg);

ReadShapeFile(gShapesOrg, gTags, sgImageDirs,
              sgTagRegex, gMask0, gMask1, sgShapeFile);
if (fgErr)                  // Err() function was called?
    return fShutdown(NULL); // error message has already been issued
ngImages = gShapesOrg.size();
if (ngImages == 0)
    {
    ReportNoImages();
    return fShutdown(NULL); // error message has already been issued
    }
lprintf("Read %d shape%s\n", ngImages, ((ngImages==1)? "":"s"));
if (gShapesOrg[0].nrows() == 0) // sanity check to track down a possible bug
    return fShutdown("First shape in the shape file has zero rows");
int iRefShape = iGetRefShapeIndex(gTags, gMask0, gMask1, sgShapeFile);
ngPoints = gShapesOrg[iRefShape].nrows();
lprintf("Reference shape %s has %d landmarks\n",
        gTags[iRefShape].c_str(), ngPoints);
DiscardShapesWithDifferentNbrOfPoints(gShapesOrg, gTags, ngPoints);
ngImages = gShapesOrg.size();
lprintf("%d shape%s\n", ngImages, ((ngImages==1)? "":"s"));
if (ngImages == 0)
    {
    ReportNoImages();
    return fShutdown(NULL); // error message has already been issued
    }
gShapes = gShapesOrg;       // vector copy
if (fgErr)                  // Err() function was called?
    return fShutdown(NULL); // error message has already been issued

if (igLandmark >= ngPoints)
    {
    lprintf("Landmark -l %d out of range, forcing to 0\n", igLandmark);
    igLandmark = 0;
    }
if (igStartLandmark >= ngPoints)
    {
    lprintf("Landmark -L %d out of range, forcing to 0\n", igStartLandmark);
    igStartLandmark = 0;
    }
if ((int)igImage >= ngImages) // can happen if shape file is not same as last time
    {
    lprintf("Image index %d out of range, forcing to 0\n", igImage);
    igImage = 0;
    }
if (sErrMsg = sLoadCurrentImage())
    return fShutdown(sErrMsg);

DisplayTitle();
ShowWindow(hgMainWnd, SW_SHOW);
UpdateWindow(hgMainWnd);
DisplayButtons();

while (GetMessage(&msg, NULL, 0, 0))
    {
    if (fgErr) // Err() function was called?
        break;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    }
if (fgErr)
    fShutdown("Early exit because of error");
else
    fShutdown(NULL);

return msg.wParam;
}
