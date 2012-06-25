// $follow.cpp 3.0 milbo$
// Derived from code by Henry Rowley http://vasc.ri.cmu.edu/NNFaceDetector.

#include "stasm.hpp"

static Mat gLightingCorrectionMat;

//-----------------------------------------------------------------------------
// This initializes the global matrix gLightingCorrectionMat.
// The lighting correction matrix is used to fit an affine function to
// the intensity values in a window.

static void
InitLightingCorrectionMat (Image &FaceMask)
{
static bool fInitialized = false;
if (!fInitialized)
    {
    fInitialized = true;
    int iPixel = 0;
    Mat mat(3,3);       // mats are automatically initialized to zero
    int width = FaceMask.width;
    int height = FaceMask.height;

    if ((width & 1) || (height & 1))  // is a problem because we use width/2 and height/2
        Err("InitLightingCorrectionMat non-even width or height");

    for (int j = -height/2; j < height - height/2; j++)
        for (int i = -width/2; i < width - width/2; i++)
            if (FaceMask(iPixel++)) // only use pixels inside the faceMask
                {
                mat(0,0) += i * i;
                mat(0,1) += i * j;
                mat(0,2) += i;
                mat(1,0) += i * j;
                mat(1,1) += j * j;
                mat(1,2) += j;
                mat(2,0) += i;
                mat(2,1) += j;
                mat(2,2) += 1;
                }

    gLightingCorrectionMat = mat.inverse();
    }
}

//-----------------------------------------------------------------------------
// Given a candidate detection location and the image pyramid, search around
// in that candidate to try and find out whether there is really a face there.
// Uses two networks (merged together), both of which have to agree on a
// detection.  If both networks agree on multiple locations, the centroid of
// those locations is computed.
// Returns true in success.

bool
fFindNewLocation (int *newX, int *newY, int *news,
                  int numScales, Image ImagePyramid[], Image &FaceMask,
                  int x, int y, int scale,
                  int dx, int dy, int ds, int step, int iNet)
{
InitLightingCorrectionMat(FaceMask);

double totX = 0;    // used to record centroid of face detections
double totY = 0;
double totS = 0;
double Total = 0;

int iPixel;
int hist[512];      // histogram
int map[512];       // cumulative histogram
Vec vec(3);         // part of affine fitting
int *tmp = new int[FaceMask.width * FaceMask.height];   // window

int halfX = FaceMask.width/2;
int halfY = FaceMask.height/2;
int nFaceMaskSize = FaceMask.width * FaceMask.height;

x += halfX;     // Input location is upper left corner of
y += halfY;     // the "centered" face.  This makes location be face center.

ForwardStruct *net = gNetList[iNet];
ForwardStruct *net2 = gNetList[iNet+1];

for (int scale1 = scale-ds; scale1 <= scale + ds; scale1++) // for each scale
    {
    // milbo: removed comment that commented out two lines below (in original
    // Rowley code) because it caused a crash because scale1 was less than 0.

    if (scale1 < 0 || scale1 >= numScales)
        continue;                                           // NOTE: continue

    int cx = (int)floor(x * POW12(scale - scale1) + 0.5);   // map center position to scale
    int cy = (int)floor(y * POW12(scale - scale1) + 0.5);

    Image *pImg = &ImagePyramid[scale1];
    for (int yy = cy - dy; yy <= cy + dy; yy += step)       // scan over position
        {
        for (int xx = cx - dx; xx <= cx + dx; xx += step)
            {
            // if (yy!=cy && xx!=cx) continue;

            iPixel = 0;
            double v0 = 0, v1 = 0, v2 = 0;

            // The next two loops copy the window into the tmp variable, and
            // begin computing the affine fit (using only pixels inside the FaceMask)
            // The first version is for windows inside the image, the second
            // replicates the edge pixels of the image.
            if (xx - halfX >= 0 && yy - halfY >= 0 &&
                    xx + halfX <= pImg->width && yy + halfY <= pImg->height)
                {
                for (int iy  = yy - halfY; iy < yy + halfY; iy++)
                    for (int ix = xx - halfX; ix < xx + halfX; ix++)
                        {
                        int val = (*pImg)(ix,iy);
                        tmp[iPixel] = val;
                        if (FaceMask(iPixel++))
                            {
                            v0 += (ix-xx) * val;
                            v1 += (iy-yy) * val;
                            v2 += val;
                            }
                        }
                }
            else
                {
                for (int iy = yy-halfY; iy<yy+halfY; iy++)
                    for (int ix = xx-halfX; ix<xx+halfX; ix++)
                        {
                        int ii = ix; if (ii < 0) ii = 0;
                        if (ii >= pImg->width) ii = pImg->width-1;
                        int jj = iy; if (jj < 0) jj = 0;
                        if (jj >= pImg->height) jj = pImg->height-1;
                        int val = (*pImg)(ii,jj);
                        tmp[iPixel] = val;
                        if (FaceMask(iPixel++))
                            {
                            v0 += (ix - xx) * val;
                            v1 += (iy - yy) * val;
                            v2 += val;
                            }
                        }
                }
            // actually compute the parameters of the affine fit

            vec(0) = v0; vec(1) = v1; vec(2) = v2;
            vec = gLightingCorrectionMat * vec;     // using overloaded operators here
            v0 = vec(0); v1 = vec(1); v2 = vec(2);

            // apply the affine correction, and build the histogram based
            // on pixels in the FaceMask

            int i;
            for (i = 0; i < 512; i++)
                hist[i] = 0;
            iPixel = 0;
            for (int j = -FaceMask.height / 2; j < FaceMask.height / 2; j++)
                for (i = -FaceMask.width / 2; i < FaceMask.width / 2; i++)
                    {
                    int val = tmp[iPixel] - (int)(i * v0 + j * v1 + v2 - 256.5);
                    if (val < 0) val = 0;
                    if (val >= 512) val = 511;
                    if (FaceMask(iPixel)) hist[val]++;
                    tmp[iPixel++] = val;
                    }
            // build the cummulative histogram

            int *to = map;
            int *from = hist;
            int total = 0;
            for (i = 0; i<512; i++)
                {
                int old = total;
                total += *(from++);
                *(to++) = old + total;
                }
            // apply histogram equalization and copy the window into the network inputs

            double scaleFactor = 1.0/total;
            ForwardUnit *pUnit = &(net->pUnitList[1]);
            int *p = tmp;
            for (i = 0; i<nFaceMaskSize; i++)
                (pUnit++)->activation = _FLOAT(map[*(p++)] * scaleFactor - 1.0);
            ForwardPass(net);

            // Check the two outputs (we applied one network, but really
            // it is two merged networks).  If both responded postively,
            // then add the detection on to the centroid of detections.

            if (net->pUnitList[net->iFirstOutput].activation>0)
                {
                pUnit = &(net->pUnitList[1]);
                ForwardUnit *pUnit2 = &(net2->pUnitList[1]);
                for (i = 0; i < nFaceMaskSize; i++)
                    (pUnit2++)->activation = (pUnit++)->activation;
                ForwardPass(net2);
                if (net2->pUnitList[net2->iFirstOutput].activation>0)
                    {
                    totS += scale1;
                    totX += xx * POW12(scale1);
                    totY += yy * POW12(scale1);
                    Total++;
                    }
                }
            } // for xx
        } // for yy
    } // for scale1
delete[] tmp;
if (Total == 0)
    return false;

// compute the centroid, first in scale and then in position at that scale

int newS = iround(totS / Total);
if (newS < 0) newS = 0;
if (newS >= numScales) newS = numScales - 1;

*newX = iround(totX / (Total * POW12(newS)) - halfX);
*newY = iround(totY / (Total * POW12(newS)) - halfY);
*news = newS;

return true;
}
