// $camera.cpp 3.0 milbo$ camera functions for casm (uses the Video For Windows interface)
//
// milbo cape town nov 2008

#include <cv.h>
#include <highgui.h>
#include <string.h>

#include <windows.h>
#include <commctrl.h>
#include <vfw.h>
#include "stasm.hpp"
#include "casm.hpp"
#include "camera.hpp"

cv::VideoCapture cam(0);/*static const int        ID_CAP = 99;     // id of camera capture window*/
/*static HWND             hgCapWnd;        // camera capture window, in main window*/
static CAPSTATUS        gCapStatus;
static CAPDRIVERCAPS    gDriverCaps;

RgbImage img;

//-----------------------------------------------------------------------------
// this also init the globals gDriverCaps and gCapStatus

bool fInitCamera (HWND hMainWnd, int iDeviceIndex)
{
// Create a capture window to capture the output from the camera
// The capture window is not actually displayed (in this application)

/*hgCapWnd = capCreateCaptureWindow(NULL,
                                  WS_CHILD,         // window style
                                  0, 0, 0, 0,       // position and size
                                  hMainWnd, ID_CAP);*/

	if (!cam.isOpened()){
		Err("Can't connect to the camera.");
		return false;
	}

	/*if (!hgCapWnd)
    {
    Err("Can't open the camera display window");
    return 0;
    }
if (!capDriverConnect(hgCapWnd, iDeviceIndex))
    {
    Err("Can't connect to the camera");
    return false;
    }
if (!capDriverGetCaps(hgCapWnd, &gDriverCaps, sizeof(CAPDRIVERCAPS)))
    {
    Err("Can't get capabilities of the camera");
    return false;
    }
if (!capPreview(hgCapWnd, FALSE))   // turn off preview
    {
    Err("capPreview FALSE failed");
    return false;
    }
if (!capGetStatus(hgCapWnd, &gCapStatus, sizeof(CAPSTATUS)))
    {
    Err("Can't get status of the camera");
    return false;
    }
PrintCapStatus();*/
return true;    // success
}

//-----------------------------------------------------------------------------
void ShutdownCamera (const char sCameraImg[])
{
/*capDriverDisconnect(hgCapWnd);
lprintf("\nDelete temporary file %s\n", sCameraImg);
unlink(sCameraImg);*/
	
	cvDestroyAllWindows();
	cam.release();
}

//-----------------------------------------------------------------------------
// Write the camera image to the given file name. This is how we communicate
// the camera image to the rest of the world.

void WriteCameraImage (const char sCameraImg[])
{
/*// Call capGrabFrame twice to flush images stored in the camera queue.  This
// makes the screen image more responsive to the current position of the
// subject, especially when using slow stacked ASM models.
// TODO does this slow down the frame rate and is there a better way of doing this?

capGrabFrame(hgCapWnd);
capGrabFrame(hgCapWnd);

// now get the actual frame we want, and save it to disk

capGrabFrame(hgCapWnd);
capFileSaveDIB(hgCapWnd, sCameraImg);*/

	cv::Mat frame;
	cam.retrieve(frame);
	img = Image(frame.cols, frame.rows, false);
	memcpy(img.buf, frame.ptr(), sizeof(img));	
	cv::imwrite(sCameraImg, frame);
}

//-----------------------------------------------------------------------------
int nGetCameraWidth(void)
{
return gCapStatus.uiImageWidth;
}

//-----------------------------------------------------------------------------
int nGetCameraHeight(void)
{
return gCapStatus.uiImageHeight;
}

//-----------------------------------------------------------------------------
void PrintCapStatus (void)
{
lprintf("Camera parameters:\n");

lprintf("    uiImageWidth %d uiImageHeight %d fLiveWindow %d fOverlayWindow %d\n",
    gCapStatus.uiImageWidth, gCapStatus.uiImageHeight,
    gCapStatus.fLiveWindow, gCapStatus.fOverlayWindow);

lprintf("    fScale %d fUsingDefaultPalette %d fAudioHardware %d fCapFileExists %d\n",
    gCapStatus.fScale, gCapStatus.fUsingDefaultPalette,
    gCapStatus.fAudioHardware, gCapStatus.fCapFileExists);

// lprintf("    dwCurrentVideoFrame %d dwCurrentVideoFramesDropped %d\n",
//     gCapStatus.dwCurrentVideoFrame, gCapStatus.dwCurrentVideoFramesDropped);
// lprintf("    dwCurrentWaveSamples %d dwCurrentTimeElapsedMS %d\n",
//     gCapStatus.dwCurrentWaveSamples, gCapStatus.dwCurrentTimeElapsedMS);
// lprintf("    hPalCurrent %lx fCapturingNow %d dwReturn %d\n",
//     gCapStatus.hPalCurrent, gCapStatus.fCapturingNow, gCapStatus.dwReturn);
// lprintf("    wNumVideoAllocated %d wNumAudioAllocated %d\n",
//     gCapStatus.wNumVideoAllocated, gCapStatus.wNumAudioAllocated);
}

//-----------------------------------------------------------------------------
/*void IdmCaptureFormat (void)
{
if (!gDriverCaps.fHasDlgVideoFormat)
    Err("The camera driver does not support this command");
else if (capDlgVideoFormat(hgCapWnd))
    capGetStatus(hgCapWnd, &gCapStatus, sizeof(CAPSTATUS));
}

//-----------------------------------------------------------------------------
void IdmCaptureSource (void)
{
if (!gDriverCaps.fHasDlgVideoSource)
    Err("The camera driver does not support this command");
else if (capDlgVideoSource(hgCapWnd))
    capGetStatus(hgCapWnd, &gCapStatus, sizeof(CAPSTATUS));
}*/
