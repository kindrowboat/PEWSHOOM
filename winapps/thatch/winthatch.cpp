// $winthatch.cpp 3.0 milbo$ windows program to generate thatcher faces from webcam
//
// milbo petaluma apr 2010

#include <windows.h>
#include <commctrl.h>
#include <vfw.h>
#include <process.h>
#include "stasm.hpp"
#include "libthatch.hpp"
#include "winthatch.hpp"
#include "../casm/camera.hpp"
#include "winfile.hpp"

// Note: version numbers which must be updated before a new release:
//      winthatch.cpp: WINTHATCH_VERSION below
//      stasm/installers/WinThatch/make.bat
//      stasm/installers/WinThatch/winthatch.iss
//      doc/stasm-manual.bat
//      homepage/stasm/index.html
// Also update the Changes list in Winthatch-readme.html

static const char   *sgWebAddr =        // used in help window
                        "http://www.milbo.users.sonic.net/stasm";

// Location of the WinThatch README file
// TODO following will only work if WinThatch is installed in the standard place
// so it won't work for non english locales with a different "Program Files"

static const char   *sgReadmeAddr =
                        "\"\\Program Files\\WinThatch\\stasm\\WinThatch-readme.html\"";

static const char *WINTHATCH_VERSION = "0.3";
       const char *sgProgramName   = "WinThatch"; // non static so Err() can display it
static const char *sgDispClass     = "disp";
static const char *sgRegistryKey   = "Software\\\\WinThatch";
static const char *sgRegistryName  = "Config";
static const int  ngToolbarHeight  = 28;     // nbr pixels toolbar takes on screen

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

static HINSTANCE hgAppInstance;         // application instance
static HWND      hgWnd;                 // main window
static HWND      hgDispWnd;             // display window (in main window)
static HWND      hgToolbar;             // toolbar
static HBITMAP   hgToolbarBmp;          // bitmap to display in the toolbar
static char      sgCameraImg[SLEN];     // filename of bmp from camera
static bool      fgCameraInitialized;   // set true after the camera is initialized
static bool      fgAsmInitialized;      // set true after the first call to AsmSearch
static bool      fgShutdown;            // true when shutting down this program
static bool      fgMinimized;           // true when window is minimized
static int       igDeviceIndex = 0;     // -d flag, camera device index
static bool      fgUseRegEntries=true;  // -F flag
static bool      fgAdvanced = false;    // -A flag
static int       igFaceNotFound;        // counter to prevent flashing "Face not found" message

// Data that is shared between AsmThread and the main program.
// This must be carefully read and written to, to prevent races.

static volatile bool fgDataInUseByAsmThread; // flags to prevent read/write races
static volatile bool fgDataInUseByMainThread;
static RgbImage      gCameraImg;    // image got from camera
static SHAPE         gAsmShape;     // ASM shape, in synch with gCameraImg
static DET_PARAMS    gDetParams;    // face detector box

static RgbImage gDispImg;           // image currently displayed on the screen

static char sgUsage[] =
"Usage: WinThatch [FLAGS]\n"
"\n"
"-d N\tCamera device index (default is 0)\n"
"-A\tAdvanced (display experimental and debugging buttons)\n"
"-F\tFresh start (ignore saved registry entries for window positions etc.)\n";

// toolbar buttons
static bool fgFlipImage = true;   // show image upside down
static bool fgFreeze;             // freeze image
static bool fgShowCamera;         // true to show the camera (no thatcherize)
static bool fgShowLandmarks;      // show results of ASM search, for debugging
static bool fgShowBoxes;          // show bounding boxes
static bool fgCropToFace;
static bool fgThirdEye;
static bool fgMirrorMouth;        // mirror mouth horizontally when thatcherizing
static bool fgShiftFeatures;      // shift other features (as wellas flipping eyes and mouth)
static double gExpandThatch = 1;  // expand thatcherized areas features by this amount

static bool fgAngleNoThatch;        // don't thatcherize if the face is at an angle
static double gAngle;               // current face angle (estimated from landmarks)
const static double gMaxAngle = 5;  // if angle bigger than this then don't thaccherize

static TBBUTTON gToolbarButtons[] =
    {
    12, IDM_FlipImage,     BUTTON_Standard,
    7,  IDM_Freeze,        BUTTON_Standard,
    0,  IDM_ToggleCamera,  BUTTON_Standard,
    11, IDM_WriteImage,    BUTTON_Standard,
    1,  IDM_CapSource,     BUTTON_Standard,
    2,  IDM_CapFormat,     BUTTON_Standard,
    5,  IDM_Help,          BUTTON_Standard,
    -1, // -1 terminates list
    };

static TBBUTTON gAdvancedToolbarButtons[] =
    {
    12, IDM_FlipImage,     BUTTON_Standard,
    7,  IDM_Freeze,        BUTTON_Standard,
    0,  IDM_ToggleCamera,  BUTTON_Standard,
    11, IDM_WriteImage,    BUTTON_Standard,
    1,  IDM_CapSource,     BUTTON_Standard,
    2,  IDM_CapFormat,     BUTTON_Standard,
    5,  IDM_Help,          BUTTON_Standard,
    0,  0,                 TBSTATE_ENABLED, TBSTYLE_SEP, 0, 0, 0, 0, // separator
    6,  IDM_ShowLandmarks, BUTTON_Standard,
    14, IDM_ShowBoxes,     BUTTON_Standard,
    8,  IDM_CropToFace,    BUTTON_Standard,
    17, IDM_ThirdEye,      BUTTON_Standard,
    16, IDM_MirrorMouth,   BUTTON_Standard,
    10, IDM_ShiftFeatures, BUTTON_Standard,
    15, IDM_AngleNoThatch, BUTTON_Standard,
    9,  IDM_ExpandThatch,  BUTTON_Standard,
    -1, // -1 terminates list
    };


char *sgTooltips[] =
    {
    "Rotate image",                                     // IDM_FlipImage
    "Freeze image",                                     // IDM_Freeze
    "Show camera (no thatcherize)",                     // IDM_ToggleCamera
    "Write displayed image to disk",                    // IDM_WriteImage
    "Set camera parameters",                            // IDM_CapSource
    "Set camera format",                                // IDM_CapFormat
    "Help",                                             // IDM_Help
    "Show landmarks",                                   // IDM_ShowLandmarks
    "Show bounding boxes",                              // IDM_ShowBoxes
    "Crop to face",                                     // IDM_CropToFace
    "Add third eye (with no thatcherizing)",            // IDM_ThirdEye
    "Mirror the mouth horizontally when thatcherizing", // IDM_MirrorMouth
    "Randomly shift other face features",               // IDM_ShiftFeatures
    "Don't thatcherize if face at an angle",            // IDM_AngleNoThatch
    "Expand size of thatcherized areas",                // IDM_ExpandThatch
    };

//-----------------------------------------------------------------------------
static LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK DispWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static void AsmThread(PVOID pParams);
static void ShutdownWin();
static void CreateToolbar(HWND hWnd);
static void UpdateTitle(void);
static void WindowMsg(char *pArgs, ...);
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
lprintf("\n%s %s (based on stasm %s)\n", sgProgramName, WINTHATCH_VERSION, STASM_VERSION);

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
wndclass.hIcon         = LoadIcon(hInstance, "WinThatch");
wndclass.hIconSm       = LoadIcon(hInstance, "WinThatch");

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
makepath(sgCameraImg, "", sGetTempDir(), "WinThatch-temp", "bmp");
lprintf("Temporary image %s", sgCameraImg);

// get initial image to display in the display window
WriteCameraImage(sgCameraImg);               // write camera image to disk
sLoadImage(gCameraImg, sgCameraImg, false);  // read it back
InvalidateRect(hgDispWnd, NULL, false);      // trigger window repaint, will display the image

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
static void AsmThread (PVOID pParams)
{
char sDataDir[SLEN];    // data dir (for face detector files etc.)
GetDataDir(sDataDir, GetCommandLine());

char sConfFile0[SLEN], sConfFile1[SLEN]; // configuration files to specify the ASM models
sprintf(sConfFile0, "%s/%s", sDataDir, "mu-68-1d.conf");
sprintf(sConfFile1, "%s/%s", sDataDir, "mu-76-2d.conf");

while (!fgShutdown)
    {
    if (fgMinimized ||  // window is minimized, so minimize processor use
            fgFreeze)   // screen image is frozen so don't update
        {
        Sleep(100);     // 100 ms
        continue;
        }
    RgbImage CameraImg;                         // local copy of camera image
    SHAPE AsmShape;
    DET_PARAMS DetParams;

    WriteCameraImage(sgCameraImg);              // write current camera image to disk
    sLoadImage(CameraImg, sgCameraImg, false);  // read it back

    if (!fgShowCamera && !fgShutdown)
        {
        Image Img(CameraImg); // convert image to monochrome for AsmSearch
        SHAPE StartShape;
        double MeanTime;

        // TODO improve error handling here (because under windows Err does not
        // exit so if AsmSearch can't find the model file for example,
        // WinThatch will issue an error message correctly but then crash).

        AsmShape = AsmSearch(StartShape, DetParams, MeanTime,
            Img, sgCameraImg, false, sConfFile0,
            NULL,  // replace NULL with sConfFile1 for slower but slightly better locations
            sDataDir, NULL, false);

        // AsmShape.nrows() will be non zero if the facial landmarks are valid
        // (and will be 0 if the global face detector did not find a face).

        if (AsmShape.nrows())
            igFaceNotFound = 0;
         else
            igFaceNotFound++;

        fgAsmInitialized = true;
        }
    if (!fgFreeze) // test needed because if user can change fgFreeze during AsmSearch
        {
        // Update the shared data so it is available for the main thread.
        // Note that we are careful to keep the amount of code in the.
        // critical section to a minimum.
        // The while loop below prevents us writing while the main thread is reading.

        while (fgDataInUseByMainThread)
            ;
        fgDataInUseByAsmThread = true;
        gCameraImg = CameraImg;
        gAsmShape = AsmShape;
        gDetParams = DetParams;
        fgDataInUseByAsmThread = false;

        UpdateTitle();
        InvalidateRect(hgDispWnd, NULL, false);     // trigger window repaint
        }
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
DisplayButton(IDM_FlipImage,     fgFlipImage);
DisplayButton(IDM_Freeze,        fgFreeze);

if (fgAdvanced)
    {
    DisplayButton(IDM_ToggleCamera,  fgShowCamera);
    DisplayButton(IDM_ShowLandmarks, fgShowLandmarks);
    DisplayButton(IDM_ShowBoxes,     fgShowBoxes);
    DisplayButton(IDM_CropToFace,    fgCropToFace);
    DisplayButton(IDM_ThirdEye,      fgThirdEye);
    DisplayButton(IDM_MirrorMouth,   fgMirrorMouth);
    DisplayButton(IDM_ShiftFeatures, fgShiftFeatures);
    DisplayButton(IDM_AngleNoThatch, fgAngleNoThatch);
    DisplayButton(IDM_ExpandThatch,  gExpandThatch != 1);
    }
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

HDC hdc = GetDC(hgDispWnd);
RECT rect;
GetClientRect(hgDispWnd, &rect);
rect.left = (2 * rect.left + rect.right) / 3;
rect.top = 10;
SetBkMode(hdc, OPAQUE);
SetBkColor(hdc, RGB(0, 0, 0));
SetTextColor(hdc, RGB(255,255,255));
DrawText(hdc, sMsgWithSpaces, -1, &rect, 0);
ReleaseDC(hgDispWnd, hdc);
}


//-----------------------------------------------------------------------------
// Possibly put a message on top of the image, e.g. "Freeze"

static void DisplayCurrentState (void)
{
char s[SLEN]; s[0] = 0;

char sMirrorMouth[SLEN]; sMirrorMouth[0] = 0;
if (fgMirrorMouth)
    sprintf(sMirrorMouth, " [Mirror mouth]");

char sShiftFeatures[SLEN]; sShiftFeatures[0] = 0;
if (fgShiftFeatures)
    sprintf(sShiftFeatures, " [Shift other features]");

char sAngleNoThatch[SLEN]; sAngleNoThatch[0] = 0;
if (fgAngleNoThatch && igFaceNotFound == 0)
    {
    if (ABS(gAngle) <= gMaxAngle)
        sprintf(sAngleNoThatch, " [Angle %.0f]", gAngle);
    else
        sprintf(sAngleNoThatch, " [Angle %.0f no thatcherize]", gAngle);
    }
char sExpand[SLEN]; sExpand[0] = 0;
if (gExpandThatch != 1)
    sprintf(sExpand, " [Expand %.2f]", gExpandThatch);

if (fgFreeze)
    {
    if (fgShowCamera)
        strcpy(s, "Freeze camera (no thatcherization)");
    else
        strcpy(s, "Freeze");
    }
else if (!fgCameraInitialized)
    strcpy(s, "Initializing camera");
else if (fgShowCamera)
    strcpy(s, "Camera (no thatcherization)");
else if (!fgAsmInitialized)
    strcpy(s, "Initializing face detector ...");
// by checking igFaceNotFound > 1 we prevent brief spurious messages when ifFaceNotFound==1
else if (!fgShowCamera && igFaceNotFound > 1)
    strcpy(s, "Face not found");

if (s[0] || sMirrorMouth[0] || sShiftFeatures[0] || sAngleNoThatch[0] || sExpand[0])
    WindowMsg("%s%s%s%s%s", s, sMirrorMouth, sShiftFeatures, sAngleNoThatch, sExpand);
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

int yDest = 0;
int nDestHeight = (rect.bottom - rect.top);
if (fgFlipImage)
    {
    yDest = rect.bottom - rect.top;
    nDestHeight = rect.top - rect.bottom;
    }
StretchDIBits(hdc,
    0,                      // xDest
    yDest,                  // yDest
    rect.right - rect.left, // nDestWidth
    nDestHeight,            // nDestHeight
    0,                      // xSrc
    0,                      // ySrc
    Img.width,              // nSrcWidth
    Img.height,             // nSrcHeight
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

// Copy the data created by AsmThread (gAsmShape and gCameraImg).
// The while below loop prevents us reading while AsmThread is writing.

while (fgDataInUseByAsmThread)
    ;
fgDataInUseByMainThread = true;
gDispImg = gCameraImg;      // copy of image that was used for the last ASM search
SHAPE AsmShape(gAsmShape);  // ASM shape from last ASM search
DET_PARAMS DetParams = gDetParams;
fgDataInUseByMainThread = false;

// AsmShape.nrows() will be non zero if the facial landmarks are valid
// (and will be 0 if the global face detector did not find a face).

if (!fgShowCamera && AsmShape.nrows())
    {
    gAngle = 0;
    if (DetParams.lex != INVALID && DetParams.ley != INVALID) // eyes ok?
        {
        gAngle = -180 / M_PI *
                 atan((DetParams.rey - DetParams.ley) / (DetParams.rex - DetParams.lex));
        if (!fgAngleNoThatch || ABS(gAngle) <= gMaxAngle)
            {
            if (fgThirdEye)
                AddThirdEye(gDispImg, AsmShape, gExpandThatch, fgShowBoxes);
            else
                Thatcherize(gDispImg, AsmShape, gExpandThatch,
                    fgMirrorMouth, fgShiftFeatures, fgShowBoxes);
            }
        }
    if (fgShowLandmarks)
        {
        DrawShape(gDispImg, DetParamsToShape(DetParams), C_YELLOW, true, C_YELLOW);
        DrawShape(gDispImg, AsmShape, C_RED, true, C_DRED);
        }
    if (fgCropToFace)
        CropImageToShape(gDispImg, AsmShape);
    }
DisplayImage(hdc, hWnd, gDispImg);
DisplayCurrentState(); // possibly put a message on top of the image
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
        sprintf(s1, "WinThatch %s", WINTHATCH_VERSION);
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
static void WriteImage (HWND hWnd)
{
char sPath[SLEN];
GetSaveBmpFilename(sPath, hWnd);  // get sPath from the user
if (sPath[0])                     // valid path?
    {
    RgbImage Img(gDispImg);
    if (fgFlipImage)
        FlipImage(Img);
    WriteBmp(Img, sPath, VERBOSE);
    char s[SLEN]; sprintf(s, "Wrote %s", sPath);
    MessageBox(hWnd, s, sgProgramName, MB_OK|MB_ICONINFORMATION);
    }
}

//-----------------------------------------------------------------------------
static void ToggleCamera (void)
{
fgShowCamera ^= 1;
fgFreeze = false;
if (fgShowCamera)
    {
    // clear the ASM shape so when we change from showing the camera to
    // showing the ASM, we don't momentarily show a stale ASM shape
    while (fgDataInUseByAsmThread)
        ;
    fgDataInUseByMainThread = true;
    gAsmShape.free();
    fgDataInUseByMainThread = false;
    }
}

//-----------------------------------------------------------------------------
// a button in the toolbar was pressed

static void WmCommand (HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
switch (wParam)
    {
    case IDM_FlipImage:
        fgFlipImage ^= 1;
        break;
    case IDM_Freeze:
        fgFreeze ^= 1;
        break;
    case IDM_ToggleCamera:
        ToggleCamera();
        break;
    case IDM_WriteImage:
        WriteImage(hWnd);
        break;
    case IDM_CapSource:
        IdmCaptureSource();
        UpdateTitle();
        break;
    case IDM_CapFormat:
        IdmCaptureFormat();
        UpdateTitle();
        break;
    case IDM_Help:
        if (DialogBox(hgAppInstance, "HelpDlg", hWnd, HelpProc) < 0)
            Err("DialogBox failed");
        break;
    case IDM_ShowLandmarks:
        fgShowLandmarks ^= 1;
        break;
    case IDM_ShowBoxes:
        fgShowBoxes ^= 1;
        break;
    case IDM_CropToFace:
        fgCropToFace ^= 1;
        break;
    case IDM_ThirdEye:
        fgThirdEye ^= 1;
        fgFreeze = false;
        if (fgThirdEye)
            fgShowCamera = false;
        break;
    case IDM_ExpandThatch:
        gExpandThatch *= 1.1;
        if (gExpandThatch < 1)
            gExpandThatch = 1;
        if (gExpandThatch > 1.4)
            gExpandThatch = .9;
        break;
    case IDM_MirrorMouth:
        fgMirrorMouth ^= 1;
        break;
    case IDM_ShiftFeatures:
        fgShiftFeatures ^= 1;
        break;
    case IDM_AngleNoThatch:
        fgAngleNoThatch ^= 1;
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
static void WmKeydown (HWND hWnd, WPARAM wParam, LPARAM lParam)
{
bool fRedisplay = true;

if(lParam & 0x0200000) switch (wParam) // translate function key
    {
    case VK_F1: wParam = 'r'; break;
    case VK_F2: wParam = 'f'; break;
    case VK_F3: wParam = 'c'; break;
    }
switch (wParam)
    {
    case 'r':     // rotate (flip)
    case 'R':
        fgFlipImage ^= 1;
        break;
    case 'f':     // freeze 46
    case 'F':
        fgFreeze ^= 1;
        break;
    case 'c':     // show camera (no thatcherization)
    case 'C':
        ToggleCamera();
        break;
    default:
        fRedisplay = false; // silently ignore unrecognized keys
        break;
    }
if (fRedisplay)
    {
    // repaint windows (causes WM_PAINT message which get handled elsewhere)

    InvalidateRect(hgWnd, NULL, false);
    InvalidateRect(hgDispWnd, NULL, false);

    fgErr = false;  // hack: see comment in WmCommand
    }
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

    case WM_KEYDOWN:        // user hit a key
        WmKeydown(hWnd, wParam, lParam);
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

TBBUTTON *pToolbarButtons = gToolbarButtons;
if (fgAdvanced)
    pToolbarButtons = gAdvancedToolbarButtons;

int nButtons;
for (nButtons = 0; nButtons < 100; nButtons++)       // calculate nButtons
    if (pToolbarButtons[nButtons].iBitmap < 0)
        break;

hgToolbar = CreateToolbarEx(
        hWnd,                   // parent window
        WS_VISIBLE|TBSTYLE_FLAT|TBSTYLE_TOOLTIPS,
        1,                      // child ID of the this toolbar child window
        1,                      // nBitmaps
        NULL,                   // hBmInst
        (UINT)hgToolbarBmp,     // wBMID
        pToolbarButtons,        // lpButtons
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
else
    DisplayTitle("%s %s             %dx%d",
                 sgProgramName, WINTHATCH_VERSION,
                 nGetCameraWidth(), nGetCameraHeight());
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
bool  fUseDefaults = !fgUseRegEntries;

// get values from registry and validate them

if (!fUseDefaults)
    {
    if (ERROR_SUCCESS !=
            RegOpenKeyEx(HKEY_CURRENT_USER, sgRegistryKey, 0, KEY_READ, &hKey) ||
        ERROR_SUCCESS !=
            RegQueryValueEx(hKey, sgRegistryName, NULL, NULL, Buf, &nBuf) ||
        4 != sscanf((char *)Buf, "%d %d %d %d",
                    &xPos, &yPos, &xSize, &ySize))
        {
        fUseDefaults = true;
        }
    RegCloseKey(hKey);
    }
RECT RectWorkArea;
SystemParametersInfo(SPI_GETWORKAREA, 0, &RectWorkArea, 0);

if (fUseDefaults ||
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
    }
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

sprintf(sRegValue, "%ld %ld %ld %ld %d",
    rectMain.left, rectMain.top,
    rectMain.right-rectMain.left, rectMain.bottom-rectMain.top);

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
            case 'A':
                fgAdvanced = true;
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
