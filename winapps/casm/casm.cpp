// $casm.cpp 3.0 milbo$ run ASM on images captured from a web cam
//
// milbo cape town nov 2008

#include <windows.h>
#include <commctrl.h>
#include <vfw.h>
#include <process.h>
#include "stasm.hpp"
#include "casm.hpp"
#include "camera.hpp"
#include "winfile.hpp"

static const char *CASM_VERSION   = "version 3.0";
       const char *sgProgramName  = "Casm"; // non static so Err() can display it
static const char *sgDispClass    = "disp";
static const char *sgRegistryKey  = "Software\\\\Casm";
static const char *sgRegistryName = "Config";
static const int  ngToolbarHeight = 28;     // nbr pixels toolbar takes on screen
static const int  ngTimeTick      = 50;     // milliseconds

// web address for help window
static const char *sgWebAddr = "http://www.milbo.users.sonic.net/stasm";

// Location of the Casm README file
// TODO following will only work if casm is installed in the standard place
// so it won't work for non english locales with a different "Program Files"

static const char *sgReadmeAddr =
                        "\"\\Program Files\\Casm\\stasm\\Casm-readme.html\"";

// There are three windows in this program:
//
//   (1) The main window hgWnd which has the two child windows listed below.
//
//   (2) The display window hgDispWnd.  This covers the whole client area of
//   the main window, apart from the toolbar area. We display the face in
//   this window.  The window procedure for the display window is DispWndProc.
//
//   (3) The capture window hgCapWnd.  The capture window is not visible but is
//   needed by the VFW interface (in camera.cpp) to get the image from the
//   camera. All of that is handled in camera.cpp.
//   We get the image from the capture window by calling WriteCameraImage
//   i.e. the camera communicates to the rest of the program via a file
//   (called gCamerImg, the "g" means global).
//   Note that we don't need to provide a window procedure for the capture window.
//
// There are two threads in this program:
//
//   (1) The main window thread (WinMain), which handles the user interface
//   and displaying images etc.
//
//   (2) The ASM thread (AsmThread), which gets images from the camera and
//   runs the ASM search.  The ASM search is slow and would block the user
//   interface if it was in the same thread as the main window. The threads
//   communicate via shared variables, see below.

static HINSTANCE hgAppInstance;      // application instance
static HWND      hgWnd;              // main window
static HWND      hgDispWnd;          // display window (in main window)
static HWND      hgToolbar;          // toolbar
static HBITMAP   hgToolbarBmp;       // bitmap to display in the toolbar
static char      sgCameraImg[SLEN];  // filename of bmp from camera
static bool      fgCameraInitialized;// set true after the camera is initialized
static bool      fgAsmInitialized;   // set true after the first call to AsmSearch
static bool      fgShutdown;         // true when shutting down this program
static bool      fgMinimized;        // true when window is minimized
static int       igDeviceIndex = 0;  // -d flag, camera device index
static bool      fgUseRegEntries = true; // -H flag

// data that is shared between AsmThread and the main program
static volatile bool  fgDataInUseByAsmThread; // flags to prevent read/write races
static volatile bool  fgDataInUseByMainThread;
static Image      gCameraImg;       // image got from camera (in monochrome)
static SHAPE      gAsmShape;        // ASM shape
static double     gMeanFrameTime;   // mean time through AsmLoop i.e. mean frame time

static char sgUsage[] =
"Usage: casm [FLAGS]\n"
"\n"
"-d N\tCamera device index (default is 0)\n"
"-F\tFresh start (ignore saved registry entries for window positions etc.)\n";

// toolbar buttons
static bool      fgShowCamera;
static bool      fgStackedModels;
static bool      fgPaleface;
static bool      fgCrop;
static bool      fgConnectDots = true;
static bool      fgShowNbrs;

static TBBUTTON gToolbarButtons[] =
    {
    26, IDM_ShowCamera,     BUTTON_Standard,
    33, IDM_StackedModels,  BUTTON_Standard,
    24, IDM_Crop,           BUTTON_Standard,
    25, IDM_Paleface,       BUTTON_Standard,
    8,  IDM_ConnectDots,    BUTTON_Standard,
    10, IDM_ShowNbrs,       BUTTON_Standard,
    27, IDM_CapSource,      BUTTON_Standard,
    28, IDM_CapFormat,      BUTTON_Standard,
    17, IDM_Help,           BUTTON_Standard,
    -1, // -1 terminates list
    };

char *sgTooltips[] =
    {
    "Show camera",                                   // IDM_ShowCamera
    "Use stacked models (slower but more accurate)", // IDM_StackedModels
    "Crop image to the face",                        // IDM_Crop
    "Lighten image",                                 // IDM_Paleface
    "Connect the dots",                              // IDM_ConnectDots
    "Show landmark numbers",                         // IDM_ShowNbrs
    "Set camera parameters",                         // IDM_CapSource
    "Set camera format",                             // IDM_CapFormat
    "Help",                                          // IDM_Help
    };

//-----------------------------------------------------------------------------
static LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK DispWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static void AsmThread(PVOID pParams);
static void ShutdownWin();
static void CreateToolbar(HWND hWnd);
static void UpdateTitle(void);
static void WindowMsg(const COLORREF Color, const char sMsg[]);
static void GetStateFromRegistry(int *pxPos, int *pyPos, int *pxSize, int *pySize);
static void SaveStateToRegistry(HWND hMainWnd);
static char *sProcessCmdLine(LPSTR pCmdLine);

//-----------------------------------------------------------------------------
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
// check if this program is running already
CreateMutex(NULL, TRUE, sgProgramName);
if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
    Err("%s is already running", sgProgramName);
    return 0;
    }
hgAppInstance = hInstance;

if (char *sErrMsg = sProcessCmdLine(szCmdLine))
    {
    Err(sErrMsg);
    return 0;
    }
lprintf("\n%s %s (based on stasm %s)\n", sgProgramName, CASM_VERSION, STASM_VERSION);

WNDCLASSEX wndclass;
wndclass.lpszClassName = sgProgramName;
wndclass.cbSize        = sizeof(wndclass);
wndclass.style         = CS_HREDRAW | CS_VREDRAW;
wndclass.lpfnWndProc   = WndProc;
wndclass.cbClsExtra    = 0;
wndclass.cbWndExtra    = 0;
wndclass.hInstance     = hInstance;
wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
wndclass.lpszMenuName  = NULL;
wndclass.hIcon         = LoadIcon(hInstance, "casm");
wndclass.hIconSm       = LoadIcon(hInstance, "casm");

if (!RegisterClassEx(&wndclass))
    Err("RegisterClass for %s failed", sgProgramName);

int xPos, yPos, xSize, ySize;
GetStateFromRegistry(&xPos, &yPos, &xSize, &ySize);

hgWnd = CreateWindow(sgProgramName,
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

if (!hgWnd)
    Err("CreateWindowEx failed");

wndclass.lpszClassName = sgDispClass;
wndclass.lpfnWndProc   = DispWndProc;
wndclass.hIcon         = NULL;
wndclass.hIconSm       = NULL;

if (!RegisterClassEx(&wndclass))
    Err("RegisterClass for %s failed", sgDispClass);

hgDispWnd = CreateWindow(sgDispClass,
        NULL,                       // window caption
        WS_CHILDWINDOW|WS_VISIBLE,  // window style
        0,                          // x position
        ngToolbarHeight,            // y position
        xSize,                      // x size
        ySize - ngToolbarHeight,    // y size
        hgWnd,                      // parent window handle
        NULL,                       // window menu handle
        hInstance,                  // program instance handle
        NULL);                      // creation parameters

if (!hgDispWnd)
    Err("CreateWindowEx failed");

CreateToolbar(hgWnd);
ShowWindow(hgWnd, iCmdShow);
ShowWindow(hgDispWnd, iCmdShow);
UpdateTitle();

if (!fInitCamera(hgWnd, igDeviceIndex))
   return 0;
fgCameraInitialized = true;
UpdateTitle();

// get temporary filename used for saving camera images
makepath(sgCameraImg, "", sGetTempDir(), "casm-temp", "bmp");
lprintf("Temporary image %s", sgCameraImg);

// get initial image to display in the display window
WriteCameraImage(sgCameraImg);                     // write camera image to disk
sLoadImage(gCameraImg, sgCameraImg, false, false); // read it back
InvalidateRect(hgDispWnd, NULL, false);                  // trigger window repaint, will display the image

_beginthread(AsmThread, 0, NULL);            // start ASM searches in parallel

MSG msg;
while (GetMessage(&msg, NULL, 0, 0))
    {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    }
ShutdownWin();
return msg.wParam;
}

//-----------------------------------------------------------------------------
// TODO should use an STL queue instead of my home grown queue?

static double GetRunningMean (double Time)
{
static const int ngQueueSize = 10;
static double Q[ngQueueSize];
static int iQ;

// insert Time into the queue

Q[iQ] = Time;
if (++iQ >= ngQueueSize)
    iQ = 0;

// sum elements of the queue

double Mean = 0;
for (int i = 0; i < ngQueueSize; i++)
    Mean += Q[i];

return Mean / ngQueueSize;
}

//-----------------------------------------------------------------------------
static void AsmThread (PVOID pParams)
{
char sDataDir[SLEN];    // data dir (for face detector files etc.)
GetDataDir(sDataDir, GetCommandLine());

char sConfFile0[SLEN], sConfFile1[SLEN];
sprintf(sConfFile0, "%s/%s", sDataDir, "mu-68-1d.conf");
sprintf(sConfFile1, "%s/%s", sDataDir, "mu-76-2d.conf");

double StartTime = clock();

while (!fgShutdown)
    {
    if (fgMinimized)    // window is minimized, so minimize processor use
        {
        Sleep(ngTimeTick);
        continue;
        }
    SHAPE AsmShape;
    SHAPE StartShape;
    DET_PARAMS DetParams;
    double MeanTime;
    Image Img;

    WriteCameraImage(sgCameraImg);              // write current camera image to disk
    sLoadImage(Img, sgCameraImg, false, false); // read camera image back from disk

    if (!fgShowCamera && !fgShutdown)
        {
        // We use sConfFile1a to do the following:
        // Call AsmShape with both models the first time, to initialize
        // both models.  Thereafter call AsmSearch with the second model
        // only if fgStackedModels is true.

        const char *sConfFile1a = sConfFile1;
        if (fgAsmInitialized && !fgStackedModels)
            sConfFile1a = NULL;

        // TODO fix error handling here (because under windows Err does not
        // exit so if AsmSearch can't find the model file for example,
        // casm will issue an error message correctly but then crash).

        AsmShape = AsmSearch(StartShape, DetParams, MeanTime,
                             Img, sgCameraImg, false,
                             sConfFile0, sConfFile1a,
                             sDataDir, NULL, false);

        if (AsmShape.nrows() > 68)  // TODO revist, needed for mu-76-2d.conf
            AsmShape.dimKeep(68, 2);

        fgAsmInitialized = true;
        }
    double LoopTime = clock() - StartTime;
    StartTime = clock();
    double MeanFrameTime = GetRunningMean(LoopTime);

    // Update the shared data so it is available for the main thread.
    // Note that we are careful to keep the amount of code in the.
    // critical section to a minimum.
    // The while loop below prevents us writing while the main thread is reading.

    while (fgDataInUseByMainThread)
        ;
    fgDataInUseByAsmThread = true;
    gCameraImg = Img;
    gAsmShape = AsmShape;
    gMeanFrameTime = MeanFrameTime;
    fgDataInUseByAsmThread = false;

    UpdateTitle();
    InvalidateRect(hgDispWnd, NULL, false);     // trigger window repaint

    // if showing the camera or if the face detector failed then
    // pause briefly to slow down this while loop to reduce processor use

    if (fgShowCamera || AsmShape.nrows() == 0)
        Sleep(ngTimeTick);
    }
_endthread();
}

//-----------------------------------------------------------------------------
static void ShutdownWin (void)
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
ShutdownCamera(sgCameraImg);
ShutdownStasm();
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
DisplayButton(IDM_ShowCamera,    fgShowCamera);
DisplayButton(IDM_StackedModels, fgStackedModels);
DisplayButton(IDM_Crop,          fgCrop);
DisplayButton(IDM_Paleface,      fgPaleface);
DisplayButton(IDM_ConnectDots,   fgConnectDots);
DisplayButton(IDM_ShowNbrs,      fgShowNbrs);
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
static void DisplayImage (HDC hdc, HWND hWnd, const RgbImage &Img)
{
BITMAPINFO BmInfo;

memset(&BmInfo.bmiHeader, 0, sizeof(BmInfo.bmiHeader));
BmInfo.bmiHeader.biSize     = 40;
BmInfo.bmiHeader.biWidth    = Img.width;
BmInfo.bmiHeader.biHeight   = Img.height;
BmInfo.bmiHeader.biPlanes   = 1;
BmInfo.bmiHeader.biBitCount = 24;

SetStretchBltMode(hdc, COLORONCOLOR);

RECT rect;
GetClientRect(hWnd, &rect);

StretchDIBits(hdc,
    0, 0,
    rect.right - rect.left, rect.bottom - rect.top,
    0, 0,                   // xSrcLowerLeft, ySrcLowerLeft
    Img.width,              // SrcWidth
    Img.height,             // SrcHeight
    Img.buf,                // lpBits
    (LPBITMAPINFO)&BmInfo,  // lpBitsInfo
    DIB_RGB_COLORS,         // wUsage
    SRCCOPY);               // raser operation code
}

//-----------------------------------------------------------------------------
// This is invoked when a WM_PAINT message is sent to the display window.
// Thus this is usually triggered by the call to InvalidateRect
// in AsmThread, but it will also be invoked if the user changes the
// window sizes etc.

static void WmDispPaint (HWND hWnd)
{
PAINTSTRUCT ps;
HDC hdc = BeginPaint(hWnd, &ps);

// Copy the data created by AsmThread gAsmShape and gCameraImg.
// The while below loop prevents us reading while AsmThread is writing.
while (fgDataInUseByAsmThread)
    ;
fgDataInUseByMainThread = TRUE;
RgbImage Img(gCameraImg);         // image that was used for the last ASM search
SHAPE AsmShape(gAsmShape);  // ASM shape from last ASM search
fgDataInUseByMainThread = FALSE;

if (fgPaleface)
    LightenImage(Img);

// AsmShape.nrows() will be non zero if the facial landmarks are valid
// (and will be 0 if the global face detector did not find a face).

if (!fgShowCamera && AsmShape.nrows())
    {
    DrawShape(Img, AsmShape, C_RED, fgConnectDots, C_DRED, fgShowNbrs);
    if (fgCrop)
        CropImageToShape(Img, AsmShape);
    }
DisplayImage(hdc, hWnd, Img);
unsigned Color = RGB(255, 255, 0);  // yellow;
if (!fgCameraInitialized)
    WindowMsg(RGB(0, 0, 0), "Initializing camera");
else if (fgShowCamera)
    WindowMsg(Color, "Camera");
else if (!fgAsmInitialized)
    WindowMsg(Color, "Initializing ASM ...");
else if (fgStackedModels)
    WindowMsg(Color, "Stacked models");

EndPaint(hWnd, &ps);
}

//-----------------------------------------------------------------------------
static LRESULT CALLBACK DispWndProc (HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
switch(iMsg)
    {
    case WM_PAINT:
        WmDispPaint(hWnd);
        return 0;
    }
return DefWindowProc(hWnd, iMsg, wParam, lParam);
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
        sprintf(s1, "Casm %s", CASM_VERSION);
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
// a button in the toolbar was pressed

static void WmCommand (HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
switch (wParam)
    {
    case IDM_ShowCamera:
        fgShowCamera ^= 1;
        // clear the ASM shape so when we change from showing the camera to
        // showing the ASM, we don't momentarily show a stale ASM shape
        while (fgDataInUseByAsmThread)
            ;
        fgDataInUseByMainThread = TRUE;
        gAsmShape.free();
        fgDataInUseByMainThread = FALSE;
        break;
    case IDM_StackedModels:
        fgStackedModels ^= 1;
        break;
    case IDM_Crop:
        fgCrop ^= 1;
        break;
    case IDM_Paleface:
        fgPaleface ^= 1;
        break;
    case IDM_ConnectDots:
        fgConnectDots ^= 1;
        break;
    case IDM_ShowNbrs:
        fgShowNbrs ^= 1;
        break;
    case IDM_CapSource:
        //IdmCaptureSource();
        UpdateTitle();
        break;
    case IDM_CapFormat:
        //IdmCaptureFormat();
        UpdateTitle();
        break;
    case IDM_Help:
        if (DialogBox(hgAppInstance, "HelpDlg", hWnd, HelpProc) < 0)
            Err("DialogBox failed");
        break;
    default:
        Err("WmCommand bad param %u", wParam);
        break;
    }
// repaint windows (causes WM_PAINT message which get handled elsewhere)

InvalidateRect(hgWnd, NULL, false);
InvalidateRect(hgDispWnd, NULL, false);

// Hack: clear global error flag that was possibly set in a previous call to Err
// (If there was a call to Err, the user would have seen a popup error message.)
// This is needed because in a Windows environment we don't exit on Err (as we
// do in a command line environment).  Various code in stasm checks fgErr and
// aborts if it is set.  So we don't want previous errors to cause problems
// now i.e. start with a clean slate after any of the above IDM commands.

fgErr = false;
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
static LRESULT CALLBACK WndProc (HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
switch(iMsg)
    {
    case WM_PAINT:
        DisplayButtons();
        break;

    case WM_SIZE:             // user changed the window's size
        SendMessage(hgToolbar, TB_AUTOSIZE, 0, 0L);
        MoveWindow(hgDispWnd, // resize child window too
                   0, ngToolbarHeight,
                   LOWORD(lParam), HIWORD(lParam) - ngToolbarHeight,
                   true);
        return 0;

    case WM_COMMAND:        // toolbar button was clicked
        WmCommand(hWnd, iMsg, wParam, lParam);
        return 0;

    case WM_CLOSE:
        fgShutdown = true;
        UpdateTitle();
        SaveStateToRegistry(hgWnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_NOTIFY:         // mouse is over a button so display a tooltip
        WmNotify(hWnd, iMsg, wParam, lParam);
        return 0;

    case WM_ACTIVATE:
        fgMinimized = (HIWORD(wParam)) != 0;
        break;
    }
return DefWindowProc(hWnd, iMsg, wParam, lParam);
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
static void DisplayTitle (char *pArgs, ...)
{
va_list pArg;
char sMsg[SLEN];

va_start(pArg, pArgs);
vsprintf(sMsg, pArgs, pArg);
va_end(pArg);

SetWindowText(hgWnd, sMsg);
}

//-----------------------------------------------------------------------------
// update main window title (it's the text in the top of the window frame)

static void UpdateTitle (void)
{
if (fgShutdown)
    DisplayTitle("Shutting down ...");  // shutdown can be slow so pacify user
else if (fgShowCamera)
    DisplayTitle("%s %s     %dx%d",
                 sgProgramName, CASM_VERSION,
                 nGetCameraWidth(), nGetCameraHeight());
else
    DisplayTitle("%s %s     Time %d ms     %dx%d",
                 sgProgramName, CASM_VERSION,
                 10 * iround(gMeanFrameTime / 10),  // round to 10ms
                 nGetCameraWidth(), nGetCameraHeight());
}

//-----------------------------------------------------------------------------
// Write given string to the given window

static void WindowMsg (const COLORREF Color,
                       const char sMsg[])  // write msg onto the screen
{
HDC hdc = GetDC(hgDispWnd);
RECT rect;
GetClientRect(hgDispWnd, &rect);
SetBkMode(hdc, TRANSPARENT);
SetTextColor(hdc, Color);
char s[SLEN];
sprintf(s, "%s", sMsg);
DrawText(hdc, s, -1, &rect, 0);
ReleaseDC(hgDispWnd, hdc);
}

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
HKEY  hKey = NULL;
DWORD nBuf = SLEN;
BYTE  Buf[SLEN];
int   xPos, yPos, xSize, ySize;
// we use int instead of bool for these because you can't scanf a bool
int   fShowCamera, fPaleface, fCrop, fConnectDots, fShowNbrs, fStackedModel;
bool  fUseDefaults = !fgUseRegEntries;

// get values from registry and validate them

if (!fUseDefaults)
    {
    if (ERROR_SUCCESS !=
            RegOpenKeyEx(HKEY_CURRENT_USER, sgRegistryKey, 0, KEY_READ, &hKey) ||
        ERROR_SUCCESS !=
            RegQueryValueEx(hKey, sgRegistryName, NULL, NULL, Buf, &nBuf) ||
        10 != sscanf((char *)Buf, "%d %d %d %d %d %d %d %d %d %d",
                    &xPos, &yPos, &xSize, &ySize,
                    &fShowCamera, &fStackedModel, &fCrop, &fConnectDots,
                    &fShowNbrs, &fPaleface))
        {
        fUseDefaults = true;
        }
    RegCloseKey(hKey);
    }
RECT RectWorkArea;
SystemParametersInfo(SPI_GETWORKAREA, 0, &RectWorkArea, 0);

if (fUseDefaults ||
    !fBool(fShowCamera)     ||
    !fBool(fStackedModel)   ||
    !fBool(fCrop)           ||
    !fBool(fConnectDots)    ||
    !fBool(fShowNbrs)       ||
    !fBool(fPaleface)       ||
    xSize > RectWorkArea.right - RectWorkArea.left ||
    xSize < 20 ||
    ySize > RectWorkArea.bottom - RectWorkArea.top ||
    ySize < 20 ||
    xPos + xSize < RectWorkArea.left + 10 ||
    yPos < RectWorkArea.top - 20)
    {
    // use defaults

    lprintf("Using default settings\n");
    xSize = (RectWorkArea.right - RectWorkArea.left) / 3;
    ySize = int(.75 * xSize + ngToolbarHeight);
    xPos = RectWorkArea.right/2 - xSize/2;
    yPos = RectWorkArea.top;
    fShowCamera = fStackedModel = fShowNbrs = fPaleface = fCrop = false;
    fConnectDots = true;
    }
fgShowCamera = (fShowCamera == 1);
fgStackedModels = (fStackedModel == 1);
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

GetWindowRect(hMainWnd, &rectMain);

sprintf(sRegValue, "%ld %ld %ld %ld %d %d %d %d %d %d",
    rectMain.left, rectMain.top,
    rectMain.right-rectMain.left, rectMain.bottom-rectMain.top,
    fgShowCamera, fgStackedModels, fgCrop, fgConnectDots, fgShowNbrs, fgPaleface);

// no point in checking return values in func calls below

RegCreateKeyEx(HKEY_CURRENT_USER, sgRegistryKey, 0, "", 0,
    KEY_ALL_ACCESS, NULL, &hKey, &dwDispo);
RegSetValueEx(hKey, sgRegistryName, 0, REG_SZ, (CONST BYTE *)sRegValue,
    strlen(sRegValue)+1);
RegCloseKey(hKey);
}

//-----------------------------------------------------------------------------
// return err msg if failure, NULL on success

static char *sProcessCmdLine (LPSTR pCmdLine)
{
const char *sWhiteSpace = " \t";
char *sToken = strtok(pCmdLine, sWhiteSpace);
while (sToken != NULL)
    {
    if (sToken[0] == '-')
        {
        if (sToken[1] == 0)
            return sgUsage;
        switch (sToken[1])
            {
            case 'd':
                sToken = strtok(NULL, sWhiteSpace);
                if (sToken == NULL)
                    return sgUsage;
                igDeviceIndex = -1;
                sscanf(sToken, "%d", &igDeviceIndex);
                if (igDeviceIndex < 0 || igDeviceIndex > 20)
                    return sgUsage;
                break;
            case 'F':
                fgUseRegEntries = false;
                break;
            default:    // bad flag
                return sgUsage;
            }
        }
    else
        return sgUsage;
    sToken = strtok(NULL, sWhiteSpace);
    }
return NULL;    // success
}
