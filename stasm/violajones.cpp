// $violajones.cpp 3.0 milbo$ interface to OpenCV Viola Jones face detector.
//
// TODO We re-open the Img in ViolaJonesFindFace, rather than using the
// passed-in Img parameter.  This wastes a bit of time.
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

#if _MSC_VER==1200 /* Visual C 6.0 */
  #include "stasm_cv.h"
#else
  #include "cv.h"
  #include "highgui.h"
#endif
#include "stasm.hpp"

// accurate but slow: these values are recommended in opencvref_cv.htm
static const double SCALE_FACTOR   = 1.1;
static const int    MIN_NEIGHBORS  = 3;
static const int    DET_FLAGS = 0;
static const bool   GET_LARGEST_VJ_FACE = 1;

static const char *CASCADE_NAME = "haarcascade_frontalface_alt2.xml";

static CvHaarClassifierCascade *pgCascade;
static CvMemStorage *pgStorage;

//-----------------------------------------------------------------------------
// specifying a min face width helps reduce nbr of false positives

static int nMinFaceWidth (int ImgW)
{
return ImgW / 4;
}

//-----------------------------------------------------------------------------
void CloseViolaJones (void)
{
if (pgCascade)
    {
    cvReleaseHaarClassifierCascade(&pgCascade);
    cvReleaseMemStorage(&pgStorage);
    pgCascade = NULL;
    }
}

//-----------------------------------------------------------------------------
static void OpenViolaJones (const char sDataDir[]) // init VJ data
{
char sCascadePath[SLEN];
sprintf(sCascadePath, "%s/%s", sDataDir, CASCADE_NAME);
pgCascade = (CvHaarClassifierCascade*)cvLoad(sCascadePath, 0, 0, 0);
if(!pgCascade)
    Err("can't load %s", sCascadePath);
pgStorage = cvCreateMemStorage(0);
if (!pgStorage)
    Err("out of memory (cvCreateMemStorage failed)");
}

//-----------------------------------------------------------------------------
// Auxilary function for fFindViolaJonesFace.
// Returns true with face position in DetParams if successful.
// Else returns false with DetParams untouched.
// In any case, eye fields in DetParams are not touched.

static bool
fViolaJonesFindFace1 (DET_PARAMS &DetParams,    // out
                      const char sImage[],      // in: image name
                      const char sDataDir[])    // in
{
if (!pgCascade) // first time? if so, must init detector data structures
    OpenViolaJones(sDataDir);
cvClearMemStorage(pgStorage);

IplImage* pImage = cvLoadImage(sImage, CV_LOAD_IMAGE_GRAYSCALE);
if (!pImage)
    Err("can't load %s", sImage);
cvResize(pImage, pImage, CV_INTER_LINEAR);
cvEqualizeHist(pImage, pImage);

CvSeq* pFaces = cvHaarDetectObjects(pImage, pgCascade, pgStorage,
                    SCALE_FACTOR, MIN_NEIGHBORS, DET_FLAGS,
                    cvSize(nMinFaceWidth(pImage->width),
                    nMinFaceWidth(pImage->width)));

if (pFaces == NULL)     // should never happen
    Err("cvHaarDetectObjects failed");

if (pFaces->total)  // success?
    {
    int iSelectedFace = 0;
    if (GET_LARGEST_VJ_FACE) // get largest face?
        {
        double MaxWidth = -1;
        for (int iFace = 0; iFace < pFaces->total; iFace++)
            {
            double Width = ((CvRect*)cvGetSeqElem(pFaces, iFace))->width;
            if (Width > MaxWidth)
                {
                MaxWidth = Width;
                iSelectedFace = iFace;
                }
            }
        }
    else // get most central face
        {
        double MaxOffset = FLT_MAX; // max abs dist from face center to image center
        for (int iFace = 0; iFace < pFaces->total; iFace++)
            {
            CvRect* r = (CvRect*)cvGetSeqElem(pFaces, iFace);
            double Offset = fabs(r->x + r->width/2.0 - pImage->width/2.0);
            if (Offset < MaxOffset)
                {
                MaxOffset = Offset;
                iSelectedFace = iFace;
                }
            }
        }
    CvRect* r = (CvRect*)cvGetSeqElem(pFaces, iSelectedFace);

    // convert x and y (OpenCV coords at corner of face box) to cartesian
    // coords (with x,y at center of face box)

    DetParams.x = r->x - pImage->width/2 + r->width/2;
    DetParams.y = pImage->height/2 - r->y - r->height/2;

    DetParams.width = r->width;
    DetParams.height = r->height;
    }
cvReleaseImage(&pImage);

return pFaces->total > 0;   // return true on success
}

//-----------------------------------------------------------------------------
// Returns true with face position in DetParams if successful.
// Else returns false with DetParams untouched.
// The eye fields of DetParams are only touched if the corresponding eye is found.

bool fFindViolaJonesFace (DET_PARAMS &DetParams,   // out
                          const Image &Img,        // in
                          const char sImage[],     // in
                          const char sDataDir[],   // in
                          const bool fEyes)        // in: find eyes too?
{
bool fSuccess = fViolaJonesFindFace1(DetParams, sImage, sDataDir);
if (fSuccess && fEyes)
    FindEyesGivenVjFace(DetParams, Img, sDataDir);
return fSuccess;
}
