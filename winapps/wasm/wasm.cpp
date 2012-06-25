// $wasm.cpp 3.0 milbo$ windows front end to stasm
//
// My windows knowledges comes from Petzold's books, so that's where
// you want to look to understand this program, if necessary.
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
//-----------------------------------------------------------------------------

#include <windows.h>
#include <commctrl.h>
#include "stasm.hpp"
#include "wasm.hpp"
#include "winfile.hpp"

#define WC_BLACK RGB(0, 0, 0)
#define WC_WHITE RGB(255,255,255)

// sgProgramName is non static so Err() can display it

const char *sgProgramName = "Wasm";

// Note: version numbers which must be updated before a new release:
//      wasm.cpp: WASM_VERSION below
//      wasm.iss: AppVerName
//      doc/stasm-manual.bat
//      homepage/stasm/index.html
// Also update the Changes list in Wasm-readme.html

static const char   *WASM_VERSION = "version 3.0";

// Following must match the Inno Installer script wasm.iss
// because wasm.iss is setup to remove this registry entry
// when the user uninstalls wasm

static const char   *sgRegistryKey  = "Software\\\\Wasm";
static const char   *sgRegistryName = "Config";

static const char   *sgWebAddr =        // used in help window
                        "http://www.milbo.users.sonic.net/stasm";

// Location of the Wasm README file
// TODO following will only work if wasm is installed in the standard place
// so it won't work for non english locales with a different "Program Files"

static const char   *sgReadmeAddr =
                        "\"\\Program Files\\Wasm\\stasm\\Wasm-readme.html\"";

static const char   *sgLogBaseName      = "Wasm-log.txt";   // log filename
static char         sgLogFile[SLEN];    // expanded log filename
static HWND         hgMainWnd;          // main window
static HWND         hgToolbar;          // toolbar
static HBITMAP      hgToolbarBmp;       // toolbar bitmap
static HINSTANCE    hgAppInstance;      // application instance
static RgbImage     gImg;               // current image
static RgbImage     gImgWithShape;      // current image with shape drawn
static char         sgFile[SLEN];       // filename of current image
static DET_PARAMS   gDetParams;         // face detector parameters
static SHAPE        gStartShape;        // start shape
static SHAPE        gShape;             // located landmarks
static char         sgDataDir[SLEN];    // dir for face detector files etc.
static char         sgConfFile0[SLEN];  // first config file, see Init()
static char         sgConfFile1[SLEN];  // second config file
static int          ngMainWidth;        // main window dimensions
static int          ngMainHeight;
static bool         fgSearching;        // true while in AsmSearch
static bool         fgShowStartShape;   // toolbar buttons
static bool         fgPaleface;
static bool         fgCrop = true;
static bool         fgConnectDots = true;
static bool         fgShowNbrs;

static TBBUTTON gToolbarButtons[] =
    {
    21, IDM_Open,           BUTTON_Standard,
    6,  IDM_WriteImage,     BUTTON_Standard,
    23, IDM_ShowStartShape, BUTTON_Standard,
    24, IDM_Crop,           BUTTON_Standard,
    25, IDM_Paleface,       BUTTON_Standard,
    8,  IDM_ConnectDots,    BUTTON_Standard,
    10, IDM_ShowNbrs,       BUTTON_Standard,
    17, IDM_Help,           BUTTON_Standard,
    -1, // -1 terminates list
    };

static char *sgTooltips[] =
    {
    "Open an image",                    // IDM_Open
    "Write displayed image",            // IDM_WriteImage
    "Show start shape",                 // IDM_ShowStartShape
    "Crop image to the face",           // IDM_Crop
    "Lighten image",                    // IDM_Paleface
    "Connect the dots",                 // IDM_ConnectDots
    "Show landmark numbers",            // IDM_ShowNbrs
    "Help",                             // IDM_Help
    };

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
                                  int *pxSize, int *pySize)
{
HKEY  hKey;
DWORD nBuf = SLEN;
BYTE  Buf[SLEN];
int   xPos, yPos, xSize, ySize;
// we use int instead of bool for these because you can't scanf a bool
int   fShowStartShape, fPaleface, fCrop, fConnectDots, fShowNbrs;
bool  fUseDefaults = false;

// get values from registry and validate them

if (ERROR_SUCCESS !=
        RegOpenKeyEx(HKEY_CURRENT_USER, sgRegistryKey, 0, KEY_READ, &hKey) ||
    ERROR_SUCCESS !=
        RegQueryValueEx(hKey, sgRegistryName, NULL, NULL, Buf, &nBuf))
    {
    logprintf(
        "Can't read registry HKEY_CURRENT_USER Key \"%s\" Name \"%s\"\n"
        "No problem, will use the default settings "
        "(probably running %s for the first time)\n",
        sgRegistryKey, sgRegistryName, sgProgramName);

    fUseDefaults = true;
    }
else if (9 != sscanf((char *)Buf, "%d %d %d %d %d %d %d %d %d",
                &xPos, &yPos, &xSize, &ySize,
                &fShowStartShape, &fCrop, &fConnectDots,
                &fShowNbrs, &fPaleface) ||
            xPos + xSize < 20  ||
            yPos + ySize < 20  ||
            xSize < 50 ||
            ySize < 50 ||
            !fBool(fShowStartShape) ||
            !fBool(fCrop)           ||
            !fBool(fConnectDots)    ||
            !fBool(fShowNbrs)       ||
            !fBool(fPaleface))
    {
    logprintf("Can't get values from registry HKEY_CURRENT_USER "
        "Key \"%s\" Name \"%s\"\n"
        "No problem, will use the default settings ",
        "(probably running %s for the first time)\n",
        sgRegistryKey, sgRegistryName, sgProgramName);

    fUseDefaults = true;
    }
RegCloseKey(hKey);

RECT RectWorkArea;
SystemParametersInfo(SPI_GETWORKAREA, 0, &RectWorkArea, 0);

if (fUseDefaults ||
        xSize > RectWorkArea.right - RectWorkArea.left ||
        ySize > RectWorkArea.bottom - RectWorkArea.top ||
        xPos < RectWorkArea.left ||
        yPos < RectWorkArea.top)
    {
    // use defaults

    xSize = (RectWorkArea.right - RectWorkArea.left) / 3;
    ySize = 2 * (RectWorkArea.bottom - RectWorkArea.top) / 3;
    xPos = RectWorkArea.right - xSize;
    yPos = RectWorkArea.top;
    fShowStartShape = fShowNbrs = fPaleface = false;
    fCrop = fConnectDots = true;
    }
fgShowStartShape = (fShowStartShape == 1);
fgCrop = (fCrop == 1);
fgConnectDots = (fConnectDots == 1);
fgShowNbrs = (fShowNbrs == 1);
fgPaleface = (fPaleface == 1);
*pxPos = xPos;
*pyPos = yPos;
*pxSize = xSize;
*pySize = ySize;
}

//-----------------------------------------------------------------------------
static void SaveStateToRegistry (HWND hMainWnd)
{
RECT  rectMain;
HKEY  hKey;
DWORD dwDispo;
char  sRegValue[SLEN];

logprintf("Saving options to registry: HKEY_CURRENT_USER "
          "Key \"%s\" Name \"%s\"\n",
          sgRegistryKey, sgRegistryName);

GetWindowRect(hMainWnd, &rectMain);

sprintf(sRegValue, "%ld %ld %ld %ld %d %d %d %d %d",
    rectMain.left, rectMain.top,
    rectMain.right-rectMain.left, rectMain.bottom-rectMain.top,
    fgShowStartShape, fgCrop, fgConnectDots, fgShowNbrs, fgPaleface);

// no point in checking return values in func calls below

RegCreateKeyEx(HKEY_CURRENT_USER, sgRegistryKey, 0, "", 0,
    KEY_ALL_ACCESS, NULL, &hKey, &dwDispo);
RegSetValueEx(hKey, sgRegistryName, 0, REG_SZ, (CONST BYTE *)sRegValue,
    strlen(sRegValue)+1);
RegCloseKey(hKey);
}

//-----------------------------------------------------------------------------
void CreateToolbar (HWND hWnd)
{
hgToolbarBmp = LoadBitmap(hgAppInstance, MAKEINTRESOURCE(IDR_TOOLBAR_BITMAP));
if (!hgToolbarBmp)
    Err("CreateToolbar failed, no BMP");

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
    Err("CreateToolbar failed");
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
LPNMHDR pnmh = (LPNMHDR)lParam;
if (pnmh->code == TTN_NEEDTEXT)
    {
    LPTOOLTIPTEXT pToolTipText = (LPTOOLTIPTEXT)lParam;
    int           iButton = pToolTipText->hdr.idFrom - IDM_FirstInToolbar;
    LPSTR         pDest = pToolTipText->lpszText;
    strcpy(pDest, sgTooltips[iButton]);
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
DisplayButton(IDM_ShowStartShape,   fgShowStartShape);
DisplayButton(IDM_Crop,             fgCrop);
DisplayButton(IDM_Paleface,         fgPaleface);
DisplayButton(IDM_ConnectDots,      fgConnectDots);
DisplayButton(IDM_ShowNbrs,         fgShowNbrs);
}

//-----------------------------------------------------------------------------
static void DisplayTitle (char sMsg[])
{
char s[SLEN];
if (sgFile[0] == 0)
    sprintf(s, "%s %s (based on Stasm %s)",
        sgProgramName, WASM_VERSION, STASM_VERSION);
else
    {
    const char *sBaseExt = sGetBaseExt(sgFile);
    if (sMsg && sMsg[0])
        sprintf(s, "%s %s", sBaseExt, sMsg);
    else
        sprintf(s, "%s %dx%d", sBaseExt, gImg.width, gImg.height);
    }
SetWindowText(hgMainWnd, s);
}

//-----------------------------------------------------------------------------
static bool fShapeValid (void) // true if successful AsmSearch
{
return gShape.nrows() > 0;
}

//-----------------------------------------------------------------------------
static void WindowMsg (const HWND hWnd,
                       const COLORREF Color,
                       const char sMsg[])  // write msg onto the screen
{
HDC hdc = GetDC(hWnd);
RECT rect;
GetClientRect(hWnd, &rect);
SetBkMode(hdc, TRANSPARENT);
SetTextColor(hdc, Color);
char s[SLEN];
sprintf(s, "\n\n%s", sMsg);                 // \n\n to space over toolbar
DrawText(hdc, s, -1, &rect, 0);
ReleaseDC(hWnd, hdc);
}

//-----------------------------------------------------------------------------
// display the image (with shape)

static void DisplayImage (HDC hdc, HWND hWnd)
{
BITMAPINFO BmInfo;

memset(&BmInfo.bmiHeader, 0, sizeof(BmInfo.bmiHeader));
BmInfo.bmiHeader.biSize     = 40;
BmInfo.bmiHeader.biWidth    = gImgWithShape.width;
BmInfo.bmiHeader.biHeight   = gImgWithShape.height;
BmInfo.bmiHeader.biPlanes   = 1;
BmInfo.bmiHeader.biBitCount = 24;

SetStretchBltMode(hdc, COLORONCOLOR);

StretchDIBits(hdc,
    0, 0, ngMainWidth, ngMainHeight,
    0, 0,                   // xSrcLowerLeft, ySrcLowerLeft
    gImgWithShape.width,    // SrcWidth
    gImgWithShape.height,   // SrcHeight
    gImgWithShape.buf,      // lpBits
    (LPBITMAPINFO)&BmInfo,  // lpBitsInfo
    DIB_RGB_COLORS,         // wUsage
    SRCCOPY);               // raser operation code

const COLORREF Color = (fgPaleface? WC_BLACK: WC_WHITE);
if (fgSearching)
    WindowMsg(hgMainWnd, Color, "Searching ...");
else if (!fShapeValid())    // unsuccessful AsmSearch?
    {
    char s[SLEN];
    sprintf(s, "Face not found");
    WindowMsg(hWnd, Color, s);
    }
else if (fgShowStartShape)
    WindowMsg(hWnd, Color, "Start shape");
}

//-----------------------------------------------------------------------------
static void DisplayNoImage (HDC hdc, HWND hWnd)
{
RECT rect;

// fill window with boring gray

GetClientRect(hWnd, &rect);
HBRUSH hBrush = CreateSolidBrush(RGB(100, 100, 100));
FillRect(hdc, &rect, hBrush);
DeleteObject(hBrush);
DeleteObject(SelectObject(hdc, GetStockObject (SYSTEM_FONT)));
ReleaseDC(hWnd, hdc);

if (fgSearching)
    WindowMsg(hgMainWnd, WC_WHITE, "Searching ...");
else
    WindowMsg(hWnd, WC_WHITE,
        "\nClick on the open button above to open an image\n");
}

//-----------------------------------------------------------------------------
static void WmPaint (HWND hWnd)
{
DisplayTitle("");
DisplayButtons();

PAINTSTRUCT ps;
HDC hdc = BeginPaint(hWnd, &ps);
if (sgFile[0])
    DisplayImage(hdc, hWnd);
else
    DisplayNoImage(hdc, hWnd);  // no image to display, so display gray
EndPaint(hWnd, &ps);
}

//-----------------------------------------------------------------------------
// this assumes that Img is monochrome even though it is stored as an RgbImage

static void LightenImage (RgbImage &Img)
{
for (int i = 0; i < Img.width * Img.height; i++)
    {
    int Pixel = 128 + Img(i).Red / 2;  // scale to 127 to 255
    Img(i).Red = Img(i).Green = Img(i).Blue = byte(Pixel);
    }
}

//-----------------------------------------------------------------------------
// show me17 points in yellow

static void DrawMe17Points (RgbImage &Img,                          // io
                            const SHAPE &Shape, const double Scale) // in
{
SHAPE Shape17;
ConvertToShape17(Shape17, Shape);
DrawShape(Img, Shape17 * Scale, C_YELLOW, false);
}

//-----------------------------------------------------------------------------
static void UpdateImgWithShape (void) // update gImgWithShape
{
gImgWithShape = gImg;

if (fgPaleface)
    LightenImage(gImgWithShape);

if (fShapeValid())                  // successful AsmSearch?
    {
#if 1 // scaling hack so landmark numbers are more legible
      // in small images (less text ovewrwriting)
    double Scale = 1;
    if (xShapeExtent(gShape) < 100 && fgCrop && fgShowNbrs &&
            // width must be a multiple of 4 after scaling
            gImgWithShape.width % 8 == 0)
        {
        Scale = 2;
        ScaleRgbImage(gImgWithShape,
            iround(gImgWithShape.width * Scale),
            iround(gImgWithShape.height * Scale),
            QUIET, false);
        }
#endif
    if (fgShowStartShape)
        {
        DrawShape(gImgWithShape,  gStartShape * Scale, C_RED,
                  fgConnectDots, C_DRED, fgShowNbrs);

        SHAPE DetShape = DetParamsToShape(gDetParams);
        DrawShape(gImgWithShape, DetShape * Scale, C_YELLOW, true, C_YELLOW);
#if 0
        if (CONF_fMe17) // CONF_fMe17 from conf file settings
#endif
            DrawMe17Points(gImgWithShape, gStartShape, Scale);
        }
    else
        {
        DrawShape(gImgWithShape, gShape * Scale, C_RED,
                  fgConnectDots, C_DRED, fgShowNbrs);
#if 0
        if (CONF_fMe17) // CONF_fMe17 from conf file settings
            DrawMe17Points(gImgWithShape, gShape, Scale);
#endif
        }
    if (fgCrop)
        CropImageToShape(gImgWithShape, gShape * Scale);
    }
}

//-----------------------------------------------------------------------------
static void OpenImage (HWND hWnd)
{
fgSearching = true;
InvalidateRect(hgMainWnd, NULL, false); // force repaint of entire window
const char *sErr = sLoadImage(gImg, sgFile, VERBOSE, false);
if (sErr)
    Err(sErr);
else
    {
    DesaturateRgbImage(gImg);   // convert to gray

    // Crop image so width is divisble by 4.
    // Necessary because RgbPrintf (in DrawShape) only works
    // on images with width divisible by four.
    // TODO when we do this, the printed landmark coords
    // in the log file will be off (they are for the new cropped image
    // not the original image).  Most images are divisible by 4 though.

    if (gImg.width % 4)
        {
        lprintf("Cropping image so width is divisible by 4\n");
        CropRgbImage(gImg, 0, 0, 0, 0, IM_WIDTH_DIVISIBLE_BY_4);
        }
    // TODO fix error handling here (because under windows Err does not
    // exit so if AsmSearch can't find the model file for example,
    // wasm will issue an error message correctly but then crash).

    double MeanTime;
    gShape = AsmSearch(gStartShape, gDetParams, MeanTime,
                       gImg, sgFile, false, // false means Viola Jones
                       sgConfFile0, sgConfFile1, sgDataDir, NULL, false);

    UpdateImgWithShape();       // update gImgWithShape
    }
fgSearching = false;
}

//-----------------------------------------------------------------------------
static void IdmOpen (HWND hWnd)
{
char sPath[SLEN];
GetImageFileName(sPath, hWnd);      // get sPath from the user
if (sPath[0])                       // valid filename?
    {
    strcpy(sgFile, sPath);
    OpenImage(hWnd);
    }
}

//-----------------------------------------------------------------------------
static void IdmWriteImage (HWND hWnd)       // write displayed image to disk
{
char sPath[SLEN];
GetSaveBmpFilename(sPath, hWnd);            // get sPath from the user
if (sPath[0])                               // valid path?
    {
    if (0 == _stricmp(sPath, sgFile))       // not a full test, for common err
        Err("%s is the displayed image, "
            "overwriting that is not allowed", sPath);
    else
        {
        if (fShapeValid())             // successful AsmSearch?
            WriteBmp(gImgWithShape, sPath, VERBOSE);
        else
            WriteBmp(gImg, sPath, VERBOSE);
        if (!fgErr) // no error in WriteBmp (TODO this is ugly)
            {
            char s[SLEN]; sprintf(s, "Wrote %s", sPath);
            MessageBox(hWnd, s, sgProgramName, MB_OK);
            }
        }
    }
}

//-----------------------------------------------------------------------------
static void OpenWithShell (const char s[])
{
if ((int)ShellExecute(NULL, "open", s, NULL, NULL, SW_SHOW) < 32)
    Err("could not open %s", s);
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
        sprintf(s1, "Wasm %s", WASM_VERSION);
        SetDlgItemText(hDlg, IDC_VERSION, s1);
        sprintf(s2, "Stasm %s", STASM_VERSION);
        SetDlgItemText(hDlg, IDC_STASM_VERSION, s2);
        return true;
        }
    case WM_COMMAND:
        switch(LOWORD(wParam))
            {
            case IDC_README:
                OpenWithShell(sgReadmeAddr);
                return true;
            case IDC_WEBSITE:
                OpenWithShell(sgWebAddr);
                return true;
            case IDOK:
                EndDialog(hDlg, 0);
                return true;
          }
    }
return false;
}

//-----------------------------------------------------------------------------
void WmCommand (HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
switch (wParam)
    {
    case IDM_Open:
        IdmOpen(hWnd);
        break;

    case IDM_WriteImage:
        if (!sgFile[0])
            Err("no image.  First load an image.");
        else
            IdmWriteImage(hWnd);
        break;

    case IDM_ShowStartShape:
        if (!sgFile[0])
            Err("no image.  First load an image.");
        else
            {
            fgShowStartShape ^= 1;
            UpdateImgWithShape();
            }
        break;

    case IDM_Crop:
        if (!sgFile[0])
            Err("no image.  First load an image.");
        else
            {
            fgCrop ^= 1;
            UpdateImgWithShape();
            }
        break;

    case IDM_Paleface:
        if (!sgFile[0])
            Err("no image.  First load an image.");
        else
            {
            fgPaleface ^= 1;
            UpdateImgWithShape();
            }
        break;

    case IDM_ConnectDots:
        if (!sgFile[0])
            Err("no image.  First load an image.");
        else
            {
            fgConnectDots ^= 1;
            UpdateImgWithShape();
            }
        break;

    case IDM_ShowNbrs:
        if (!sgFile[0])
            Err("no image.  First load an image.");
        else
            {
            fgShowNbrs ^= 1;
            UpdateImgWithShape();
            }
        break;

    case IDM_Help:
        if (DialogBox(hgAppInstance, "HelpDlg", hWnd, HelpProc) < 0)
            Err("DialogBox failed");
        break;

    default:
        Err("WmCommand bad param %u", wParam);
        break;
    }
InvalidateRect(hgMainWnd, NULL, false);     // force repaint of entire window

// Hack: clear global error flag that was possibly set in a previous call to Err
// (If there was a call to Err, the user would have seen a popup error message.)
// This is needed because in a Windows environment we don't exit on Err (as we
// do in a command line environment).  Various code in stasm checks fgErr and
// aborts if it is set.  So we don't want previous errors to cause problems
// now i.e. start with a clean slate after any of the above IDM commands.

fgErr = false;
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

    case WM_COMMAND:    // toolbar buttons
        WmCommand(hWnd, iMsg, wParam, lParam);
        return 0;

    case WM_CLOSE:
        SaveStateToRegistry(hgMainWnd);
        break;          // call default window close handler

    case WM_NOTIFY:     // display a tooltip
        WmNotify(hWnd, iMsg, wParam, lParam);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    }
return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
static void ShutdownStasm (void)
{
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
}

//-----------------------------------------------------------------------------
// We must open the log file in the current user's TMP directory
// so we don't have permission problems under Windows Vista

static void GetLogFilename (char sLogFile[],                // out
                            const char sLogBaseName[])      // in
{
char *sTemp = getenv("TMP");
if (sTemp)
    sprintf(sLogFile, "%s/%s", sTemp, sLogBaseName);
else    // not sure what is the best thing to do here, so use current directory
    sprintf(sLogFile, "%s", sLogBaseName);
}

//-----------------------------------------------------------------------------
static void Init (HINSTANCE hInstance, int nCmdShow)
{
WNDCLASSEX  wndclass;
hgAppInstance = hInstance;

GetDataDir(sgDataDir, GetCommandLine());

lprintf("%s %s (based on stasm %s)\n", sgProgramName, WASM_VERSION, STASM_VERSION);

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
wndclass.lpszClassName = sgProgramName;
wndclass.hIcon         = LoadIcon(hInstance, "wasm");
wndclass.hIconSm       = LoadIcon(hInstance, "wasm");

if (!RegisterClassEx(&wndclass))
    Err("RegisterClass failed");

int xPos, yPos, xSize, ySize;
GetStateFromRegistry(&xPos, &yPos, &xSize, &ySize);

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
    Err("CreateWindowEx failed");
CreateToolbar(hgMainWnd);

sprintf(sgConfFile0, "%s/%s", sgDataDir, "mu-68-1d.conf");
sprintf(sgConfFile1, "%s/%s", sgDataDir, "mu-76-2d.conf");
}

//-----------------------------------------------------------------------------
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR pCmdLine, int nCmdShow)
{
Init(hInstance, nCmdShow);
if (fgErr)  // Err() function was called?
    {
    ShutdownStasm();
    return -1;
    }
ShowWindow(hgMainWnd, SW_SHOW);
UpdateWindow(hgMainWnd);
MSG msg;
while (GetMessage(&msg, NULL, 0, 0))
    {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    }
ShutdownStasm();
return 0;
}
