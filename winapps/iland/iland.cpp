// $iland.cpp 3.0 milbo$ interactively landmark a face
//
// This is an initial implementation.  You will see
// some questionable code and a lot of "TODO" comments.
//-----------------------------------------------------------------------------

#if _WIN32_WINNT < 0x0500
#define _WIN32_WINNT 0x0500   // for WM_MOUSEWHEEL
#endif

#include <windows.h>
#include <commctrl.h>
#include <winuser.h>
#include "stasm.hpp"
#include "iland.hpp"
#include "imequalize.hpp"
#include "asmsearch1.hpp"
#include "winfile.hpp"

extern void GetPrevAndNext(int &iPrev, int &iNext,                     // out
                           int iPoint, const SHAPE &Shape);            // in

#define MOUSE_DRAG_UPDATES_MODEL 0 // TODO what is best here?

static const char   *ILAND_VERSION   = "0.3";
       const char   *sgProgramName   = "iland";
static const char   *sgClass         = "iland";
static const char   *sgRegistryKey   = "Software\\iland";
static const char   *sgRegistryName  = "Config";

// margins around the shape with igZoom==1 and igZoom==2
static const double XMARGIN = 1.0 / 6;
static const double YMARGIN = 1.0 / 5;

static const int    ngDlgWidth  = 220;
static const int    nDlgHeight = 140;
static const int    ngButtonHeight = 0; // height of button bar (TODO do this properly, should be 28)

static HWND         hgMainWnd;                  // main window
static HWND         hgToolbar;                  // toolbar
static HWND         hgDlgWnd;                   // dialog window
static HBITMAP      hgToolbarBmp;               // toolbar bitmap
static HINSTANCE    hgAppInstance;              // application instance
static int          ngMainWidth, ngMainHeight;  // main window dimensions
static int          xgDlg;                      // posn of dialog window
static int          ygDlg;

static RgbImage     gRawImg;            // raw image read from the disk
static RgbImage     gScaledImg;         // gRawImg scaled down for speed in searches
static RgbImage     gScaledImgMono;     // same as gScaledImg, but monochrome
static RgbImage     gImg;               // what we show on the screen i.e. gScaledImg with shape
static char         sgFile[SLEN];       // filename of current image
static bool         fgKeymarks[100];    // true if a point is a keymark, updated on the fly
static bool         fgInitialKeymarks[100];
static int          igLandmark;         // current landmark
static SHAPE        gShape;             // located landmarks
static SHAPE        gPrevShape;         // for sizing the displayed face

// toolbar buttons

static bool         fgConnectDots = true;
static bool         fgShowNbrs;
static bool         fgHideLandmarks = true;
static int          igZoom = 1;        // 0 no crop, 1 crop to face+margin,
                                       // 2 crop to keypoints+margin, 3..4 zoom

static int          xgMouse, ygMouse;  // mouse position
static bool         fgLMouseDown;      // true while left mouse button is down

static int          ngImgWidth, ngImgHeight;

static int igTopCrop, igBottomCrop;  // what we crop off image before displaying it
static int igLeftCrop, igRightCrop;

static char         sgDataDir[SLEN];    // dir for face detector files etc.
static char         sgConfFile0[SLEN];  // first config file, init in WinMain
static char         sgConfFile1[SLEN];  // second config file

static bool         fgUseRegEntries = true;     // -F command line flag

typedef enum eState {
    STATE_OPEN_IMAGE,     // user must open an image
    STATE_OPENING_IMAGE,  // user is busy opening an image
    STATE_SEARCHING,      // searching for landmarks
    STATE_NO_FACE,        // can't find face in image
    STATE_KEYMARKING,     // waiting for user to adjust keypoints
    STATE_LANDMARKING,    // waiting for user to adjust landmarks
    STATE_TWEAKING,       // waiting for user to tweak final landmarks
    STATE_DONE,
} eState;

static char *sgStateNames[] =
{
    "Open image",                           // STATE_OPEN_IMAGE
    "Opening image",                        // STATE_OPENING_IMAGE
    "Searching",                            // STATE_SEARCHING
    "Face not found",                       // STATE_NO_FACE
    "Phase 1: Keymarking",                  // STATE_KEYMARKING
    "Phase 2: Landmarking",                 // STATE_LANDMARKING
    "Phase 3: Tweaking",                    // STATE_TWEAKING
    "Landmarking done",                     // STATE_DONE
};

static eState gState = STATE_OPEN_IMAGE;

static TBBUTTON gToolbarButtons[] =
    {
    21, IDM_Open,               BUTTON_Standard,
    29, IDM_Blank,              BUTTON_Standard,
//      8, IDM_ConnectDots,        BUTTON_Standard,
//     10, IDM_ShowNbrs,           BUTTON_Standard,
//     28, IDM_HideLandmarks,      BUTTON_Standard,
//     29, IDM_Blank,              BUTTON_Standard,
     3, IDM_ZoomOut,            BUTTON_Standard,
     4, IDM_ZoomIn,             BUTTON_Standard,
    29, IDM_Blank,              BUTTON_Standard,
    17, IDM_Help,               BUTTON_Standard,
    -1, // -1 terminates list
    };

static char *sgTooltips[] = // order like numbers of IDM_Open etc. in iland.hpp
    {
    "Open an image",                                          // IDM_Open
    "Connect the dots\n\nKeyboard d",                         // IDM_ConnectDots
    "Show landmark numbers",                                  // IDM_ShowNbrs
    "Hide original landmarks\n\nKeyboard h",                  // IDM_HideLandmarks
    "Zoom out\n\nKeyboard -",                                 // IDM_ZoomOut
    "Zoom in\n\nKeyboard +",                                  // IDM_ZoomIn
    "Help",                                                   // IDM_Help
    "",                                                       // IDM_Blank
    };

static char sgUsage[] =
"Usage: iland [-F]\n"
"\n"
"-F   Fresh start (ignore saved registry entries for window positions etc.)\n";

static void UpdateAndDisplay(void);
static void OpenCmd(void);
static void UpdateInterface(void);

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
static void SetCrossCursor (void)
{
LPCTSTR lpCursorName = IDC_CROSS; // TODO use a custom cursor here?
if (gState == STATE_SEARCHING || gState == STATE_OPENING_IMAGE)
    lpCursorName = IDC_WAIT;
SetCursor(LoadCursor (NULL, lpCursorName));
}

//-----------------------------------------------------------------------------
static void SetArrowCursor (void)
{
LPCTSTR lpCursorName = IDC_ARROW;
if (gState == STATE_SEARCHING || gState == STATE_OPENING_IMAGE)
    lpCursorName = IDC_WAIT;
SetCursor(LoadCursor (NULL, lpCursorName));
}

//-----------------------------------------------------------------------------
// Create the same window layout as last time by looking at the registry.
// Also get toolbar button settings.
// If nothing in registry, then will init to defaults.

static void GetStateFromRegistry (int *pxPos, int *pyPos,
                int *pxSize, int *pySize, int *pxDlg, int *pyDlg)
{
HKEY  hKey;
DWORD nBuf = SLEN;
BYTE  Buf[SLEN];
int xPos, yPos, xSize, ySize;
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
            "(probably running iland for the first time)\n",
            sgRegistryKey, sgRegistryName);

        fUseDefaults = true;
        }
    else
        {
        // TODO add check that dialog window is not under the main window

        if (6 != sscanf((char *)Buf, "%d %d %d %d %d %d",
                        &xPos, &yPos, &xSize, &ySize, &xDlg, &yDlg) ||
            xPos + xSize < 20              ||
            yPos + ySize < 20              ||
            xSize < 20                     ||
            ySize < 20)
            {
            lprintf("Can't get values from registry HKEY_CURRENT_USER "
                "Key \"%s\" Name \"%s\" %dx%d %dx%d\n"
                "No problem, will use the default settings "
                "(probably running iland for the first time)\n",
                sgRegistryKey, sgRegistryName,
                xPos, yPos, xSize, ySize);

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
    xSize = (RectWorkArea.right - RectWorkArea.left) / 3;
    ySize = iround((RectWorkArea.bottom - RectWorkArea.top) / 1.5);
    xPos = RectWorkArea.right - xSize;
    yPos = RectWorkArea.top;
#if 1
    xDlg = xPos - ngDlgWidth;
    yDlg = yPos;
#else
    xDlg = xPos + 8;
    yDlg = yPos + 63;
#endif
    }
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
    "%d %d %d %d %d %d %d",
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
static void PrimeDisplayImg (void)
{
gPrevShape.dim(0, size_t(0));   // init face sizing code in CreateDisplayImg
}

//-----------------------------------------------------------------------------
static bool fGlobalShapeValid (void) // true if successful AsmSearch
{
return gShape.nrows() > 0;
}

//-----------------------------------------------------------------------------
static void DrawSquare (RgbImage &Img,           // io
                        double x, double y,      // in
                        int iSize,
                        unsigned Color)          // in: color of landmarks
{
SHAPE Square(15, 2);

Square(0, VX) = x + -iSize;  Square(0, VY) = y + -iSize;
Square(1, VX) = x +  iSize;  Square(1, VY) = y + -iSize;
Square(2, VX) = x +  iSize;  Square(2, VY) = y + iSize;
Square(3, VX) = x + -iSize;  Square(3, VY) = y + iSize;
Square(4, VX) = x + -iSize;  Square(4, VY) = y + -iSize;

Square(5, VX) = x+1 + -iSize;  Square(5, VY) = y + -iSize;
Square(6, VX) = x+1 +  iSize;  Square(6, VY) = y + -iSize;
Square(7, VX) = x+1 +  iSize;  Square(7, VY) = y + iSize;
Square(8, VX) = x+1 + -iSize;  Square(8, VY) = y + iSize;
Square(9, VX) = x+1 + -iSize;  Square(9, VY) = y + -iSize;

Square(10, VX) = x + -iSize;  Square(10, VY) = y+1 + -iSize;
Square(11, VX) = x +  iSize;  Square(11, VY) = y+1 + -iSize;
Square(12, VX) = x +  iSize;  Square(12, VY) = y+1 + iSize;
Square(13, VX) = x + -iSize;  Square(13, VY) = y+1 + iSize;
Square(14, VX) = x + -iSize;  Square(14, VY) = y+1 + -iSize;

DrawShape(Img, Square,   Color, true, Color, false, -1);
}

//-----------------------------------------------------------------------------
static void DrawBigKeymarks (void)
{
for (unsigned iPoint = 0; iPoint < gShape.nrows(); iPoint++)
    if (fgKeymarks[iPoint])
        {
        double x, y;
        GetPointCoords(x, y, iPoint, gShape);
        DrawSquare(gImg, x, y, 2, (igLandmark == iPoint? C_YELLOW: C_RED));
        }
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
DisplayButton(IDM_ConnectDots,       fgConnectDots);
DisplayButton(IDM_ShowNbrs,          fgShowNbrs);
DisplayButton(IDM_HideLandmarks,     fgHideLandmarks);
}

//-----------------------------------------------------------------------------
// Write given string to the given window

static void WindowMsg (char *pArgs, ...)   // printf style arguments
{
va_list pArg;
char sMsg[1000];    // big enough for my strings
char sMsgWithSpaces[10000];
va_start(pArg, pArgs);
vsprintf(sMsg, pArgs, pArg);
va_end(pArg);
sprintf(sMsgWithSpaces, "   %s   ", sMsg);

HDC hdc = GetDC(hgMainWnd);
RECT rect;
GetClientRect(hgMainWnd, &rect);
rect.left = (2 * rect.left + rect.right) / 3;
rect.top = 10;
SetBkMode(hdc, OPAQUE);
SetBkColor(hdc, RGB(0, 0, 0));
SetTextColor(hdc, RGB(255,255,255));
DrawText(hdc, sMsgWithSpaces, -1, &rect, 0);
ReleaseDC(hgMainWnd, hdc);
}

//-----------------------------------------------------------------------------
// display gImg (image, with possible shape in it)

static void DisplayImage (HDC hdc, HWND hWnd)
{
BITMAPINFO BmInfo;

memset(&BmInfo.bmiHeader, 0, sizeof(BmInfo.bmiHeader));
BmInfo.bmiHeader.biSize     = 40;
BmInfo.bmiHeader.biWidth    = gImg.width;
BmInfo.bmiHeader.biHeight   = gImg.height;
BmInfo.bmiHeader.biPlanes   = 1;
BmInfo.bmiHeader.biBitCount = 24;

SetStretchBltMode(hdc, COLORONCOLOR);

StretchDIBits(hdc,
    0, 0, ngMainWidth, ngMainHeight,
    0, 0,                   // xSrcLowerLeft, ySrcLowerLeft
    gImg.width,             // SrcWidth
    gImg.height,            // SrcHeight
    gImg.buf,               // lpBits
    (LPBITMAPINFO)&BmInfo,  // lpBitsInfo
    DIB_RGB_COLORS,         // wUsage
    SRCCOPY);               // raser operation code
}

//-----------------------------------------------------------------------------
// fill window with boring gray

static void DisplayNoImage (HDC hdc, HWND hWnd)
{
RECT rect;
GetClientRect(hWnd, &rect);
HBRUSH hBrush = CreateSolidBrush(RGB(100, 100, 100));
FillRect(hdc, &rect, hBrush);
DeleteObject(hBrush);
DeleteObject(SelectObject(hdc, GetStockObject (SYSTEM_FONT)));
ReleaseDC(hWnd, hdc);
}

//-----------------------------------------------------------------------------
// display the image -- magnify it so it fills the window

static void WmPaint (HWND hWnd)
{
PAINTSTRUCT ps;
HDC hdc = BeginPaint(hWnd, &ps);
if (sgFile[0])
    DisplayImage(hdc, hWnd);
else
    DisplayNoImage(hdc, hWnd);  // no image to display, so display gray
if (gState == STATE_NO_FACE)
    WindowMsg("Face not found");
else if (gState == STATE_SEARCHING)
    WindowMsg("Searching ...");
EndPaint(hWnd, &ps);
}

//-----------------------------------------------------------------------------
// This updates the user interface text on the screen.
// It also displays the toolbar buttons.

static void UpdateInterface (void)
{
// main window title
char s[SLEN];
sprintf(s, "iland     %s     zoom %d  ",  sGetBaseExt(sgFile), igZoom);
SetWindowText(hgMainWnd, s);

// dialog window title
SetWindowText(hgDlgWnd, sgStateNames[gState]);

// buttons, depressed or not
DisplayButtons();

if (!hgDlgWnd)  // dialog window not yet open?
    return;

char *sDlgText = "";
char *sBack = "Back";
char *sNext = "Next";
switch (gState)
    {
    case STATE_OPEN_IMAGE:
        sBack = "";
        sNext = "Open";
        sDlgText = "Please open an image";
        break;
    case STATE_OPENING_IMAGE:
        sBack = "";
        sNext = "";
        sprintf(s, "Opening image");
        sDlgText = s;
        break;
    case STATE_SEARCHING:
        sBack = "";
        sNext = "";
        sprintf(s, "Locating points in %s ...", sGetBaseExt(sgFile));
        sDlgText = s;
        break;
    case STATE_NO_FACE:
        sBack = "";
        sNext = "Open";
        sDlgText = "Face not found\n"
                   "\n"
                   "Please open a new image";
        break;
    case STATE_KEYMARKING:
        // sBack = "Open";    // correct, but confusing
        sDlgText = "Adjust the key points if necessary\n"
                   "and then click Next";
        break;
    case STATE_LANDMARKING:
        sDlgText = "Adjust the points if necessary\n"
                   "and then click Next";
        break;
    case STATE_TWEAKING:
        sDlgText = "Make final tweaks to\n"
                   "the individual points if necessary";
        break;
    case STATE_DONE:
        sNext = "Open";
        sDlgText = "Landmarks are ready for use";
        break;
    default:
        Err("UpdateInterface: illegal gState %d", gState);
        break;
    }
SetDlgItemText(hgDlgWnd, IDC_TEXT, sDlgText);
SetDlgItemText(hgDlgWnd, IDC_BACK, sBack);
SetDlgItemText(hgDlgWnd, IDC_NEXT, sNext);
EnableWindow(GetDlgItem(hgDlgWnd, IDC_BACK), sBack[0] != 0);
EnableWindow(GetDlgItem(hgDlgWnd, IDC_NEXT), sNext[0] != 0);
}

//-----------------------------------------------------------------------------
// This flushes mouse clicks and movements made while search is in progress.
// TODO is this what we want?

static void FlushMouseMsgs (void)
{
MSG msg;
while (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSEWHEEL, PM_REMOVE))
   ;
}

//-----------------------------------------------------------------------------
// return Shape with just the initial keymarks

static SHAPE GetShortKeyShape (SHAPE &Shape)
{
lprintf("GetShortKeyShape\n");
SHAPE KeyShape(gShape.nrows(), 2);
int iKey = 0;
for (unsigned iPoint = 0; iPoint < Shape.nrows(); iPoint++)
    if (fgInitialKeymarks[iPoint])
        {
        KeyShape(iKey, VX) = Shape(iPoint, VX);
        KeyShape(iKey, VY) = Shape(iPoint, VY);
        iKey++;
        }
return KeyShape;
}

//-----------------------------------------------------------------------------
// return Shape with gShape.nrows() points, no keymark points set to 0,0

static SHAPE GetKeyShape (void)
{
SHAPE KeyShape(gShape.nrows(), 2);
for (unsigned iPoint = 0; iPoint < gShape.nrows(); iPoint++)
    if (fgKeymarks[iPoint])
        {
        KeyShape(iPoint, VX) = gShape(iPoint, VX);
        KeyShape(iPoint, VY) = gShape(iPoint, VY);
        }
return KeyShape;
}

//-----------------------------------------------------------------------------
static bool fUserCanChangeLandmarks (void)
{
// const HWND hwnd = GetForegroundWindow();
//                     // can only change landmarks if our windows have the focus

return // (hwnd == hgMainWnd || hwnd == hgDlgWnd) &&
       (gState == STATE_KEYMARKING  ||
        gState == STATE_LANDMARKING ||
        gState == STATE_TWEAKING);
}

//-----------------------------------------------------------------------------
static void InitKeymarks (void) // init fgKeymarks and fgInitialKeymarks
{
ASSERT(NELEMS(fgKeymarks) >= gShape.nrows());
memset(fgKeymarks, 0, sizeof(fgKeymarks));
if (fGlobalShapeValid())
    {
    fgKeymarks[7]  = true;
    fgKeymarks[27] = true;
    fgKeymarks[32] = true;
    fgKeymarks[48] = true;
    fgKeymarks[54] = true;
    }
memcpy(fgInitialKeymarks, fgKeymarks, sizeof fgInitialKeymarks);
}

//-----------------------------------------------------------------------------
static void InitStateKeymarking (void)
{
gState = STATE_KEYMARKING;
UpdateInterface();
InitKeymarks(); // init fgKeymarks
fgHideLandmarks = true;
fgConnectDots = false;
fgShowNbrs = false;
}

//-----------------------------------------------------------------------------
static void InitStateLandmarking (void)
{
gState = STATE_LANDMARKING;
fgHideLandmarks = false;
fgConnectDots = true;
fgShowNbrs = false;
UpdateAndDisplay();
}

//-----------------------------------------------------------------------------
static void InitStateTweaking (void)
{
gState = STATE_TWEAKING;
UpdateAndDisplay();
}

//-----------------------------------------------------------------------------
// User clicked Next in the dialog window

static void NextCmd (void)
{
switch (gState)
    {
    case STATE_OPENING_IMAGE:
    case STATE_SEARCHING:
        break;
    case STATE_KEYMARKING:
        {
        gState = STATE_SEARCHING;
        UpdateInterface();
        // set cursor for dialog win, SetCrossCursor will set cursor for main win
        SetArrowCursor(); // TODO doesn't seem to work until you move the cursor, why?
        DET_PARAMS DetParams;   // dummy arg for AsmSearch1
        double MeanTime;        // dummy arg for AsmSearch1
        gShape = AsmSearch1(&gShape, DetParams, MeanTime, gScaledImg, sgFile, false,
                            sgConfFile0, sgConfFile1, sgDataDir, NULL,
                            false, &GetKeyShape());
        FlushMouseMsgs();
        InitStateLandmarking();
        UpdateAndDisplay();
        break;
        }
    case STATE_LANDMARKING:
        InitStateTweaking();
        UpdateAndDisplay();
        break;
    case STATE_TWEAKING:
        gState = STATE_DONE;
        UpdateAndDisplay(); // remove yellow current landmark number
        break;
    default:
        OpenCmd();
        break;
    }
}

//-----------------------------------------------------------------------------
// User clicked Back in the dialog window

static void BackCmd (void)
{
switch (gState)
    {
    case STATE_KEYMARKING:
        OpenCmd();
        break;
    case STATE_LANDMARKING:
        InitStateKeymarking();
        UpdateAndDisplay();
        break;
    case STATE_TWEAKING:
        InitStateLandmarking();
        UpdateAndDisplay();
        break;
    case STATE_DONE:
        InitStateTweaking();
        UpdateAndDisplay();
        break;
    default:
        break;
    }
}

//-----------------------------------------------------------------------------
BOOL CALLBACK DlgProc (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
switch (msg)
    {
    case WM_INITDIALOG:
        {
        // Move the dialog window to the right place.  It's actually a
        // top level window so we need to position wrt the screen

        RECT RectWorkArea;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &RectWorkArea, 0);
        MoveWindow(hDlg, xgDlg, ygDlg, ngDlgWidth, nDlgHeight+30, true);
        return false;
        }
    case WM_COMMAND:
        if (wParam == IDC_NEXT)
            NextCmd();
        else if (wParam == IDC_BACK)
            BackCmd();
        SetFocus(hgMainWnd);    // set focus back to main window
        return 1;
   case WM_SETCURSOR:
        SetArrowCursor();
        return 1;
   }
return false;
}

//-----------------------------------------------------------------------------
// If interactively marking or in JumpToLandmark mode, then keep the screen
// stable as much as possible
// TODO adjust this to be more useable?

bool fUseOldCoords (bool fJump)
{
    static int iOldLandmark;

    const bool fUseOld =
            ((igLandmark == iOldLandmark || fJump) &&
            xgMouse > 20 && xgMouse < ngMainWidth - 20 &&
            ygMouse > ngButtonHeight + 30 && ygMouse < ngMainHeight - 20);

    iOldLandmark = igLandmark;

    return fUseOld;
}

//-----------------------------------------------------------------------------
void
CropImageToShape1 (RgbImage &Img,       // io
                   int &nLeftCrop,      // out
                   int &nRightCrop,     // out
                   int &nTopCrop,       // out
                   int &nBottomCrop,    // out
                   const SHAPE &Shape)  // in
{
int Width = (int)xShapeExtent(Shape);

const double xMargin = Width * XMARGIN;
const double yMargin = Width * YMARGIN;

nLeftCrop   = iround(MAX(0, Img.width/2  + Shape.col(VX).minElem() - xMargin));
nRightCrop  = iround(MAX(0, Img.width/2  - Shape.col(VX).maxElem() - xMargin));
nTopCrop    = iround(MAX(0, Img.height/2 - Shape.col(VY).maxElem() - yMargin));
nBottomCrop = iround(MAX(0, Img.height/2 + Shape.col(VY).minElem() - yMargin));

CropRgbImage(Img,
             nTopCrop, nBottomCrop, nLeftCrop, nRightCrop,
             IM_WIDTH_DIVISIBLE_BY_4);
}

//-----------------------------------------------------------------------------
// Crop the image, depending on user setting of igZoom.
// i.e. crop gImg and set igTopCrop, igBottomCrop, igRightCrop, igLeftCrop.

static void ZoomImage (void)
{
igTopCrop = igBottomCrop = igRightCrop = igLeftCrop = 0;

if (igZoom == 0)        // don't crop
    ;
else if (igZoom == 1 || igZoom == 2)
    {
    // Crop image to shape boundaries
    // If igZoom==1 then use gShape, if igZoom==2 then use keymarks shape

    if (gPrevShape.nrows() == 0)  // first time?
        {
        CropImageToShape1(gImg, igLeftCrop, igRightCrop, igTopCrop, igBottomCrop,
                          (igZoom == 2? GetShortKeyShape(gShape): gShape));
        gPrevShape = gShape;
        }
    else
        {
        // Code to avoid excessive movement of screen image.  Basic idea: use the
        // previous image if new shape is still in the bounds of the previous image.

        SHAPE KeyShape, PrevKeyShape;
        SHAPE *pShape = &gShape, *pPrevShape = &gPrevShape;
        if (igZoom == 2)
            {
            KeyShape = GetShortKeyShape(gShape);
            PrevKeyShape = GetShortKeyShape(gPrevShape);
            pShape = &KeyShape;
            pPrevShape = &PrevKeyShape;
            }
        double xExtent = xShapeExtent(*pPrevShape);
        double xMargin = XMARGIN * xExtent - 8; // 8 extra pixels feels a bit better
        double yMargin = YMARGIN * xExtent - 8;
        if (xMargin < 0)
            xMargin = 0;
        if (yMargin < 0)
            yMargin = 0;

        double xMin = 1e10, xMax = -1e10;
        double yMin = 1e10, yMax = -1e10;
        double xPrevMin = 1e10, xPrevMax = -1e10;
        double yPrevMin = 1e10, yPrevMax = -1e10;

        for (unsigned i = 0; i < pShape->nrows(); i++)
            {
            if (fPointUsed(*pShape, i))
                {
                if ((*pShape)(i, VX) < xMin) xMin = (*pShape)(i, VX);
                if ((*pShape)(i, VX) > xMax) xMax = (*pShape)(i, VX);
                if ((*pShape)(i, VY) < yMin) yMin = (*pShape)(i, VY);
                if ((*pShape)(i, VY) > yMax) yMax = (*pShape)(i, VY);
                }
            if (fPointUsed(*pPrevShape, i))
                {
                if ((*pPrevShape)(i, VX) < xPrevMin) xPrevMin = (*pPrevShape)(i, VX);
                if ((*pPrevShape)(i, VX) > xPrevMax) xPrevMax = (*pPrevShape)(i, VX);
                if ((*pPrevShape)(i, VY) < yPrevMin) yPrevMin = (*pPrevShape)(i, VY);
                if ((*pPrevShape)(i, VY) > yPrevMax) yPrevMax = (*pPrevShape)(i, VY);
                }
            }
        if ( // test that new shape is not too big
                xMin > xPrevMin - xMargin &&
                xMax < xPrevMax + xMargin &&
                yMin > yPrevMin - yMargin &&
                yMax < yPrevMax + yMargin &&
            // test that new shape is not too small
                xMin < xPrevMin + .667 * xMargin &&
                xMax > xPrevMax - .667 * xMargin &&
                yMin < yPrevMin + .667 * yMargin &&
                yMax > yPrevMax - .667 * yMargin)
            ; // use previous shape
        else
            gPrevShape = gShape; // use new shape

        CropImageToShape1(gImg, igLeftCrop, igRightCrop, igTopCrop, igBottomCrop,
                         (igZoom == 2? GetShortKeyShape(gPrevShape): gPrevShape));
        }
    }
else    // igZoom >= 2
    {
    // TODO this works except for one thing: we need to capture the mouse
    // so the jump code works properly, or use a trick to accomplish same thing

    // Crop image with igLandmark in center of image.
    // Cropped image width is (1+XMARGIN) * xShapeExtent / igZoom.
    // i.e. if igZoom = 2 then cropped image width is half of that for igZoom = 1.

    // Some code to prevent excessive movement of the screen: use the
    // old x and y landmark positions for cropping the image where possible

    double x, y;            // point positions from gShape
    static double xOld;
    static double yOld;

    if (fUseOldCoords(true))
        {
        x = xOld;
        y = yOld;
        }
    else if (fgLMouseDown)
        {
        x = xOld;
        y = yOld;
        }
    else
        {
        // following call to GetPointCoords  is equivalent to
        // x = gShape(igLandmark, VX) and y = gShape(igLandmark, VY)
        // except that it synthesizes x and y if landmark is missing

        GetPointCoords(x, y, igLandmark, gShape);
        xOld = x;
        yOld = y;
        }
    // put igLandmark in the center of the window

    double ImgWidth = gImg.width, ImgHeight = gImg.height;
    double FaceWidth = xShapeExtent(gShape);
    double DispWidth = (1 + XMARGIN) * FaceWidth / igZoom;     // displayed image width
    double DispHeight = gImg.height * DispWidth / gImg.width;
    double Left   = x - DispWidth / 2;  // left side of image
    double Right  = x + DispWidth / 2;
    double Top    = y + DispHeight / 2;
    double Bottom = y - DispHeight / 2;

    igLeftCrop   = iround(MAX(0, Left + ImgWidth / 2));
    igRightCrop  = iround(MAX(0, ImgWidth/2 - Right));
    igTopCrop    = iround(MAX(0, ImgHeight/2 - Top));
    igBottomCrop = iround(MAX(0, Bottom + ImgHeight/2));

    CropRgbImage(gImg,
        igTopCrop, igBottomCrop, igLeftCrop, igRightCrop,
        IM_WIDTH_DIVISIBLE_BY_4);
    }
}

//-----------------------------------------------------------------------------
static void UpdateAndDisplay (void)        // this writes into gImg
{
static bool fInUpdateAndDisplay; // TODO why is this func being entered recursively?
if (fInUpdateAndDisplay)
    return;                 // recursive call to UpdateAndDisplay
fInUpdateAndDisplay = TRUE;

UpdateInterface();
DisplayButtons();
if (gState == STATE_TWEAKING || gState == STATE_DONE)
    gImg = gScaledImg;          // display color image
else
    gImg = gScaledImgMono;      // display monochrome image
if (fGlobalShapeValid())
    {
    if (!fgHideLandmarks)
        DrawShape(gImg, gShape, C_DRED,
                  fgConnectDots, C_DRED, fgShowNbrs,
                  (gState == STATE_DONE? -1: igLandmark));

    if (gState == STATE_KEYMARKING || gState == STATE_LANDMARKING)
        DrawBigKeymarks();

    ZoomImage();
    }
InvalidateRect(hgMainWnd, NULL, false);     // force repaint of entire window
SetCrossCursor();                           // this is needed, not sure why
fInUpdateAndDisplay = FALSE;
}

//-----------------------------------------------------------------------------
static void OpenCmd (void)
{
eState OldState = gState;
gState = STATE_OPENING_IMAGE;
UpdateInterface();
char sFile[SLEN];
GetImageFileName(sFile, hgMainWnd); // get sFile from the user
if (sFile[0] == 0)                  // invalid filename or user cancelled?
    {
    gState = OldState;
//     // open dialog window --- needed if the user cancels the very first file open
//     if (NULL == hgDlgWnd &&     // redundant but safe
//             NULL == (hgDlgWnd = CreateDialog(hgAppInstance, "Dlg", hgMainWnd, DlgProc)))
//            Err("CreateDialog failed");
    }
else                                // valid filename
    {
    strcpy(sgFile, sFile);
    const char *sErr = sLoadImage(gRawImg, sgFile, VERBOSE, false);  // get image from disk
    if (sErr)
        {
        Err(sErr);
        gState = OldState;
        }
    else
        {
        gState = STATE_SEARCHING;
        UpdateInterface();
        ngImgWidth = gImg.width;
        ngImgHeight = gImg.height;
        igLandmark = 27; // TODO default initial landmark
        b.dim(0, size_t(0));    // clears the global variable b
        DET_PARAMS DetParams;   // dummy arg for AsmSearch1
        double MeanTime;        // dummy arg for AsmSearch1
        // TODO if separate face detect from landmark search then can
        // speed up search by resizing large images before search
        gShape = AsmSearch1(NULL, DetParams, MeanTime, gRawImg, sgFile, false,
                            sgConfFile0, sgConfFile1, sgDataDir);
        FlushMouseMsgs();
        if (fGlobalShapeValid())
            {
            InitStateKeymarking();
            igZoom = 1;

            // scale gScaledImg to shape extent (also scale gShape to correspond)

            const double xExtent = xShapeExtent(gShape);
            ASSERT(ngModels > 0);
            const int nStandardFaceWidth = gModels[ngModels-1].nStandardFaceWidth;
            double NewWidth = gRawImg.width * nStandardFaceWidth / xExtent;
            NewWidth = (int(NewWidth) / 4) * 4;  // force width divisible by 4, needed for RgbPrintfs later
            double Scale = NewWidth / gRawImg.width;
            gShape *= Scale;
            gScaledImg = gRawImg;
            ScaleRgbImage(gScaledImg, int(NewWidth), int(Scale * gScaledImg.height), QUIET, IM_BILINEAR);
            }
        else
            {
            gScaledImg = gRawImg;
            gState = STATE_NO_FACE;
            // force image width to be divisible by 4 so we can use RgbPrintf later
            ScaleRgbImage(gScaledImg, (gScaledImg.width / 4) * 4, gScaledImg.height, QUIET, false);
            }
        gScaledImgMono = gScaledImg;
        DesaturateRgbImage(gScaledImgMono); // convert to gray
        PrimeDisplayImg();                  // init face sizing code in CreateDisplayImg
        fgLMouseDown = false;               // just in case
        }
    }
UpdateAndDisplay();

// create dialog window if this is the first time we are here
if (NULL == hgDlgWnd && // first time?
    NULL == (hgDlgWnd = CreateDialog(hgAppInstance, "Dlg", hgMainWnd, DlgProc)))
   Err("CreateDialog failed");
}

//-----------------------------------------------------------------------------
// callback procedure for the help dialog window

BOOL CALLBACK HelpProc (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
switch (msg)
    {
    case WM_INITDIALOG:
        {
        static char s1[SLEN], s2[SLEN];
        sprintf(s1, "iland %s (EXPERIMENTAL)", ILAND_VERSION);
        SetDlgItemText(hDlg, IDC_VERSION, s1);
        sprintf(s2, "Stasm %s", STASM_VERSION);
        SetDlgItemText(hDlg, IDC_STASM_VERSION, s2);
        return true;
        }
    case WM_COMMAND:
        switch(LOWORD(wParam))
            {
            case IDOK:
                EndDialog(hDlg, 0);
                return true;
          }
    }
return false;
}

//-----------------------------------------------------------------------------
static void ZoomIn (void)
{
PrimeDisplayImg();
if (++igZoom > 2)   // TODO change 2 to 4 but first fix capturing in ZoomImage
    igZoom = 2;
}

//-----------------------------------------------------------------------------
static void ZoomOut (void)
{
PrimeDisplayImg();
if (--igZoom < 0)
    igZoom = 0;
}

//-----------------------------------------------------------------------------
void WmCommand (HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
switch (wParam)
    {
    case IDM_Open:
        OpenCmd();
        break;

    case IDM_ConnectDots:
        if (fgHideLandmarks)
            {
            fgHideLandmarks = false;
            fgConnectDots = true;
            }
        else
            fgConnectDots ^= 1;
        break;

    case IDM_ShowNbrs:
        if (fgHideLandmarks)
            {
            fgHideLandmarks = false;
            fgShowNbrs = false;
            }
        else
            fgShowNbrs ^= 1;
        break;

    case IDM_HideLandmarks:
        fgHideLandmarks ^= 1;
        break;

    case IDM_ZoomOut:
        ZoomOut();
        break;

    case IDM_ZoomIn:
        ZoomIn();
        break;

    case IDM_Help:
        if (DialogBox(hgAppInstance, "HelpDlg", hWnd, HelpProc) < 0)
            Err("DialogBox failed");
        break;

    case IDM_Blank:
        break;

    default:
        Err("WmCommand bad param %u", wParam);
        break;
    }
if (wParam != IDM_Blank)
    UpdateAndDisplay();

// Hack: clear global error flag that was possibly set in a previous call to Err
// (If there was a call to Err, the user would have seen a popup error message.)
// This is needed because in a Windows environment we don't exit on Err (as we
// do in a command line environment).  Various code in stasm checks fgErr and
// aborts if it is set.  So we don't want previous errors to cause problems
// now i.e. start with a clean slate after any of the above IDM commands.

fgErr = false;
}

//-----------------------------------------------------------------------------
static void WmChar (HWND hWnd, WPARAM wParam)
{
bool fDisplay = true;
switch (wParam)
    {
    case '-':
        ZoomOut();
        break;
    case '+':
        ZoomIn();
        break;
    case 'D':                   // connect the dots
    case 'd':
        fgConnectDots ^= 1;
        break;
    case 'H':
    case 'h':
        fgHideLandmarks ^= 1;
        break;
    case 0x0d:                  // allow user to put blank lines in command window
        lprintf("\n");
        break;
    default:                    // unrecognized character, ignore it
        fDisplay = false;
    }
if (fDisplay)
    UpdateAndDisplay();
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
    x = iround(x) - 1;   // TODO why is -1 needed?
    if (x <= -gScaledImg.width / 2)         // clip to image limits?
        x = -gScaledImg.width / 2 + 1;
    else if (x >= gScaledImg.width / 2)
        x = gScaledImg.width / 2 - 1;

    return x;

}

static double yScreenToShape (double yScreen)
{
    double y = igBottomCrop -
                (gImg.height + igBottomCrop + igTopCrop) / 2 + yScreen - 0.5;
    y = iround(y);
    if (y <= -gScaledImg.height / 2)        // clip to image limits?
        y = -gScaledImg.height / 2 + 1;
    else if (y >= gScaledImg.height / 2)
        y = gScaledImg.height / 2 - 1;

    return y;
}

//-----------------------------------------------------------------------------
static int Signed (unsigned x)
{
if (x > 0x7fff)
    x -= 0x10000;
return x;
}

//-----------------------------------------------------------------------------
static void WmLButtonDown (LPARAM lParam)
{
fgLMouseDown = true;
xgMouse = Signed(LOWORD(lParam));
ygMouse = Signed(HIWORD(lParam));
double x = xScreenToShape(xMouseToScreen(xgMouse));
double y = yScreenToShape(yMouseToScreen(ygMouse));
// TODO make sure coords aren't 0,0 because that means "unused landmark"
if (x == 0 && y == 0)
    x = 0.1;
gShape(igLandmark, VX) = x;
gShape(igLandmark, VY) = y;
// capture the mouse even when mouse is off the window, Petzold Win95 p322
SetCapture(hgMainWnd);
UpdateAndDisplay();
}

//-----------------------------------------------------------------------------
static void ConformAndDisplayShape (void) // called on left mouse up
{
ASSERT(fGlobalShapeValid());
fgKeymarks[igLandmark] = true;  // fix the current point
eState OldState = gState;
gState = STATE_SEARCHING;
UpdateInterface();
ASSERT(ngModels > 0);
gShape = ConformShapeToModelWithKeymarks(b, gShape, gModels[ngModels-1], 0, true, GetKeyShape());
FlushMouseMsgs();
gState = OldState;
UpdateAndDisplay();
}

//-----------------------------------------------------------------------------
static void WmLButtonUp (LPARAM lParam)
{
fgLMouseDown = false;
ReleaseCapture();
if (gState == STATE_LANDMARKING)
    ConformAndDisplayShape();
}

//-----------------------------------------------------------------------------
// This is for jumping to the closest landmark as the user moves the mouse

static int iGetClosestLandmark (int xMouse, int yMouse)
{
double x = xScreenToShape(xMouseToScreen(xMouse));
double y = yScreenToShape(yMouseToScreen(yMouse));
double Min = 1e10;
int iBest = -1;
for (unsigned iPoint = 0; iPoint < gShape.nrows(); iPoint++)
    if (gState != STATE_KEYMARKING || fgKeymarks[iPoint])
        {
        double x1, y1;
        GetPointCoords(x1, y1, iPoint, gShape);
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
    if (!fgLMouseDown)   // prevent landmarking switching while dragging
        igLandmark = iGetClosestLandmark(xgMouse, ygMouse);
    if (MOUSE_DRAG_UPDATES_MODEL && fgLMouseDown && gState == STATE_LANDMARKING)
        ConformAndDisplayShape();
    else if (!fUseOldCoords(false))
        UpdateAndDisplay();
    if (fgLMouseDown)
        WmLButtonDown(lParam);
    }
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
    LPTOOLTIPTEXT pToolTipText = (LPTOOLTIPTEXT)lParam;
    int           iButton = pToolTipText->hdr.idFrom - IDM_FirstInToolbar;
    LPSTR         pDest = pToolTipText->lpszText;
    strcpy(pDest, sgTooltips[iButton]);
    }
}

//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc (HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
switch (iMsg)
    {
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
        SaveStateToRegistry(hgMainWnd, hgDlgWnd);
        break;  // call default window close handler

    case WM_LBUTTONDOWN:    // mouse left click, set landmark position
        if (fUserCanChangeLandmarks())
            WmLButtonDown(lParam);
        break;

    case WM_LBUTTONUP:     // mouse left click up
        if (fUserCanChangeLandmarks())
            WmLButtonUp(lParam);
        break;

    case WM_MOUSEMOVE:
        if (fUserCanChangeLandmarks())
            WmMouseMove(lParam);
        SetCrossCursor();
        break;

    case WM_CHAR:
        WmChar(hWnd, wParam);
        return 0;

    case WM_COMMAND:    // toolbar buttons
        WmCommand(hWnd, iMsg, wParam, lParam);
        return 0;

    case WM_NOTIFY:     // display a tooltip
        WmNotify(hWnd, iMsg, wParam, lParam);
        return 0;
    }
return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
static char *sCreateToolbar (HWND hWnd) // return err msg, or null if no error
{
hgToolbarBmp = LoadBitmap(hgAppInstance, MAKEINTRESOURCE(IDR_ILAND_BUTTONS));
if (!hgToolbarBmp)
    return "CreateToolbar failed, no BMP";

int nButtons;
for (nButtons = 0; nButtons < 100; nButtons++)       // calculate nButtons
    if (gToolbarButtons[nButtons].iBitmap < 0)
        break;

hgToolbar = CreateToolbarEx(
        hWnd,                   // parent window
        WS_VISIBLE|TBSTYLE_FLAT|TBSTYLE_TOOLTIPS,
        1,                      // child ID of the this toolbar child window
        1,                      // nBitmaps
        NULL,                   // hBmInst
        (UINT)hgToolbarBmp,     // wBMID
        gToolbarButtons,        // lpButtons
        nButtons,               // iNumButtons (must include Separators)
        0, 0, 0, 0,             // position etc.
        sizeof(TBBUTTON));

if (!hgToolbar)
    return "CreateTooobar failed";

return NULL;
}

//-----------------------------------------------------------------------------
char *sInitWindows (HINSTANCE hInstance, HINSTANCE hPrevInstance)
{
WNDCLASSEX  wndclass;
hgAppInstance   = hInstance;

if (pgLogFile)
    fprintf(pgLogFile, "%s %s\n", sgProgramName, ILAND_VERSION);

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
wndclass.hIcon         = LoadIcon(hInstance, "iland");
wndclass.hIconSm       = LoadIcon(hInstance, "iland");

if (!RegisterClassEx(&wndclass))
    return "RegisterClass failed";

// create the same window layout as last time by looking at registry

int xPos, yPos, xSize, ySize;
GetStateFromRegistry(&xPos, &yPos, &xSize, &ySize, &xgDlg, &ygDlg);
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
        fprintf(pgLogFile, "Msg %s\n", sMsg);
    }
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
// return err msg if failure, NULL on success

static char *sProcessCmdLine (LPSTR pCmdLine)
{
char *sToken = strtok(pCmdLine, " \t");
while (sToken != NULL)
    {
    if (sToken[0] == '-')
        {
        if (sToken[1] == 0)
            return sgUsage;
        switch (sToken[1])
            {
            case 'F':
                fgUseRegEntries = false;
                break;
            default:    // bad flag
                return sgUsage;
            }
        }
    else    // string without '-'
        return sgUsage;
    sToken = strtok(NULL, " \t");
    }
return NULL;    // success
}

//-----------------------------------------------------------------------------
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR pCmdLine, int iCmdShow)
{
MSG msg;
char *sErrMsg;

// shutdown if this program is running already
CreateMutex(NULL, true, sgProgramName);
if (GetLastError() == ERROR_ALREADY_EXISTS)
   return fShutdown("iland is running already");

if (sErrMsg = sProcessCmdLine(pCmdLine))
    {
    Err(sErrMsg);
    return fShutdown(NULL);
    }
if (sErrMsg = sInitWindows(hInstance, hPrevInstance))
    return fShutdown(sErrMsg);

GetDataDir(sgDataDir, GetCommandLine());
sprintf(sgConfFile0, "%s/%s", sgDataDir, "mu-76-2d.conf");
// sprintf(sgConfFile1, "%s/%s", sgDataDir, "mu-76-2d.conf"); // TODO

// never use thick lines in DrawShape
extern int igThickenShapeWidth; igThickenShapeWidth = 10000;

ShowWindow(hgMainWnd, SW_SHOW);
UpdateWindow(hgMainWnd);
DisplayButtons();
OpenCmd();
while (GetMessage(&msg, NULL, 0, 0))
    {
    if (fgErr)  // Err() function was called?
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
