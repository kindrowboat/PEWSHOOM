// $search.cpp 3.0 milbo$
// Derived from code by Henry Rowley http://vasc.ri.cmu.edu/NNFaceDetector.

#include "stasm.hpp"
#include "list.cpp"

//-----------------------------------------------------------------------------
class AccumElement : public ListNode<AccumElement>
{
public:
int loc[5];
int value;
AccumElement(int x, int y, int scale, int d, int t, int v)
    {
    loc[0] = x; loc[1] = y; loc[2] = scale; loc[3] = d; loc[4] = t;
    value = v;
    next = NULL;
    }
};

//-----------------------------------------------------------------------------
// Get the value of some location in the detection pyramid.  If that location
// does not exist, they return zero (but do not allocate that location).

static int
FuzzyVoteAccumCheck (Tcl_HashTable *accum,
                     int x, int y, int scale)
{
int loc[3];
loc[0] = x; loc[1] = y; loc[2] = scale;
Tcl_HashEntry *entry = Tcl_FindHashEntry(accum,(char*)loc);
if (entry == NULL)
    return 0;
return ((AccumElement*)(Tcl_GetHashValue(entry)))->value;
}

//-----------------------------------------------------------------------------
// Set a value in the detection pyramid; if that location does not exist,
// then create it.  Also, add that location to the list of locations with
// that value.

static void
FuzzyVoteAccumSet (Tcl_HashTable *accum, List<AccumElement> *bins,
                   int x, int y, int scale, int val)
{
int loc[3];
loc[0] = x; loc[1] = y; loc[2] = scale;
Tcl_HashEntry *entry = Tcl_FindHashEntry(accum,(char*)loc);
AccumElement *elem = (AccumElement*)(Tcl_GetHashValue(entry));
bins[elem->value].unchain(elem);
if (val>255)
    val = 255;
bins[val].addLast(elem);
elem->value = val;
}

//-----------------------------------------------------------------------------
// Set a value in the detection pyramid to zero.  If the location does not
// exist, then do not create it (because the default value is zero).

static void
FuzzyVoteAccumZero (Tcl_HashTable *accum, List<AccumElement> *bins,
                    int x, int y, int scale)
{
int loc[3];
loc[0] = x; loc[1] = y; loc[2] = scale;
Tcl_HashEntry *entry = Tcl_FindHashEntry(accum,(char*)loc);
if (entry == NULL)
    return;
AccumElement *elem = (AccumElement*)(Tcl_GetHashValue(entry));
bins[elem->value].unchain(elem);
bins[0].addLast(elem);
elem->value = 0;
}

//-----------------------------------------------------------------------------
// Get the value of some location in the detection pyramid.  If that location
// has not already been allocated, create it and set it to zero.

static int
FuzzyVoteAccumGet (Tcl_HashTable *accum, List<AccumElement> *bins,
                   int x, int y, int scale)
{
int loc[3];
loc[0] = x; loc[1] = y; loc[2] = scale;
Tcl_HashEntry *entry = Tcl_FindHashEntry(accum,(char*)loc);
if (entry == NULL)
    {
    int newentry;
    entry = Tcl_CreateHashEntry(accum,(char*)loc,&newentry);
    AccumElement *elem = new AccumElement(x,y,scale,0,0,0);
    Tcl_SetHashValue(entry,elem);
    bins[0].addLast(elem);
    }
return ((AccumElement*)(Tcl_GetHashValue(entry)))->value;
}

//-----------------------------------------------------------------------------
// Given some point in the detection pyramid, locate all 6-connection
// locations with a value greater than or equal to the specified amount,
// and find their centroid.  The locations in the centroid are set to zero.
// Centroid in scale is computed by averaging the pyramid levels at which
// the faces are detected.

static void
FindCentroidAccum (Tcl_HashTable *accum, List<AccumElement> *bins,
                   int numImages,
                   int scale, int x, int y, int val,
                   double *totalS, double *totalX, double *totalY,
                   double *total)
{
int value = FuzzyVoteAccumCheck(accum,x,y,scale);
if (value>=val)
    {
    FuzzyVoteAccumZero(accum,bins,x,y,scale);
    (*total) += value;
    (*totalS) += scale*value;
    (*totalX) += x*POW12(scale)*value;
    (*totalY) += y*POW12(scale)*value;
    FindCentroidAccum(accum,bins,numImages,scale,x+1,y,val,
                      totalS,totalX,totalY,total);
    FindCentroidAccum(accum,bins,numImages,scale,x-1,y,val,
                      totalS,totalX,totalY,total);
    FindCentroidAccum(accum,bins,numImages,scale,x,y+1,val,
                      totalS,totalX,totalY,total);
    FindCentroidAccum(accum,bins,numImages,scale,x,y-1,val,
                      totalS,totalX,totalY,total);
    FindCentroidAccum(accum,bins,numImages,
                      scale-1,(int)floor(0.5+x*1.2),(int)floor(0.5+y*1.2),val,
                      totalS,totalX,totalY,total);
    FindCentroidAccum(accum,bins,numImages,
                      scale+1,(int)floor(0.5+x/1.2),(int)floor(0.5+y/1.2),val,
                      totalS,totalX,totalY,total);
    }
}

//-----------------------------------------------------------------------------
// Implementation of the arbitration mechanisms described in the paper.
// Inputs are lists of detections, the width and height of the image that
// is being processed, a callback function used to report the arbitration
// results, and a bunch of parameters for the arbitration itself.

void
FuzzyVote2 (int width, int height, int nLocFiles, List<Detection> Detections[],
            CALLBACK_FUN callbackFunc, ClientData callbackData,
            int spread, int search,
            bool fCollapseNearbyOverlappingDetections,
            bool fRemoveOverlappingImages,
            bool fFilterOdd,
            Image &Mask)
{
// Hash table represents which locations/scales in the detection are
// filled in, and with what values.  Any missing entries are assumed to be zero.

Tcl_HashTable accum;
Tcl_InitHashTable(&accum, 3);

// An array of lists.  Each lists contains all the detection location/scales
// with a particular value.  This allows the Detections to be scanned in
// order from highest to lowest with practically no overhead.

List<AccumElement> bins[256];

// figure out the number of scales in the pyramid

int nTempWidth = width, nTempHeight = height;
int nScales = 0;
while (nTempWidth >= Mask.width && nTempHeight >= Mask.height)
    {
    nTempWidth = (int)(floor(nTempWidth/1.2));
    nTempHeight = (int)(floor(nTempHeight/1.2));
    nScales++;
    }
// for each detection list given as input (all detections are treated equally)
for (int num = 0; num < nLocFiles; num++)
    {
    int nFaces = 0;

    // for each detection

    for (Detection *detect = Detections[num].first; detect != NULL; detect = detect->next)
        {
        int x = detect->x, y = detect->y, scale = detect->scale;
        if (fFilterOdd && ((x & 1) || (y & 1)))
            continue;

        int xc = x + Mask.width/2;
        int yc = y + Mask.height/2;

        // Spread out the detection in both scale and location by
        // "spread" levels or pixels, incrementing the value of each
        // location in the detection pyramid.

        for (int si = -spread; si <= spread; si++)
            {
            if (si + scale < 0 || si + scale >= nScales)
                continue;
            int xci = (int)floor(0.5 + xc * exp(log(1.2) * -si));
            int yci = (int)floor(0.5 + yc * exp(log(1.2) * -si));
            // int sspread = (int)floor(0.5+spread*POW12(-si));
            int sspread = spread;
            for (int xx = xci-sspread; xx <= xci + sspread; xx++)
                for (int yy = yci-sspread; yy <= yci + sspread; yy++)
                    FuzzyVoteAccumSet(&accum, bins, xx, yy, scale + si,
                        FuzzyVoteAccumGet(&accum, bins, xx, yy, scale + si) + 1);
            }
        nFaces++;
        }
    }
// scan through the detection pyramid from highest to lowest value

for (int val = 255; val >= search; val--)
    {
    while (!bins[val].empty())
        {
        // get the detection

        int x = bins[val].first->loc[0];
        int y = bins[val].first->loc[1];
        int scale = bins[val].first->loc[2];
        int cs, cx, cy;

        if (fCollapseNearbyOverlappingDetections)
            {
            // collapse nearby overlapping detections by finding the
            // centroid of the detections around this location

            double total = 0.0;
            double totalS = 0, totalX = 0, totalY = 0;
            FindCentroidAccum(&accum, bins, nScales, scale, x, y, search,
                &totalS, &totalX, &totalY, &total);
            cs = (int)floor(0.5 + totalS / total);
            cx = (int)floor(0.5 + totalX / total * exp(log(1.2) * -cs));
            cy = (int)floor(0.5 + totalY / total * exp(log(1.2) * -cs));
            }
        else
            {
            // record this location

            cs = scale;
            cx = x;
            cy = y;
            FuzzyVoteAccumZero(&accum, bins, x, y, scale);
            }

        if (fRemoveOverlappingImages)
            {
            // scan through the detection pyramid, removing all possible overlapping images

            for (scale = 0; scale < nScales; scale++)
                {
                int xpos = (int)floor(0.5 + cx * POW12(cs - scale));
                int ypos = (int)floor(0.5 + cy * POW12(cs - scale));
                int sizex = Mask.width / 2 +
                                (int)floor(0.5 + Mask.width / 2 * POW12(cs - scale));
                int sizey = Mask.height/2 +
                                (int)floor(0.5 + Mask.height / 2 * POW12(cs - scale));
                for (int xx = xpos-sizex; xx <= xpos + sizex; xx++)
                    for (int yy = ypos-sizey; yy <= ypos + sizey; yy++)
                        FuzzyVoteAccumZero(&accum, bins, xx, yy, scale);
                }
            }

        // record (or otherwise deal with) this detection

        callbackFunc(callbackData, NULL,                 // callbackData, image
            cx - Mask.width / 2, cy - Mask.height / 2,   // x, y
            Mask.width, Mask.height, cs,                 // width, height, level
            POW12(cs), 1.0);                             // scale, output, orientation
        }
    }
Tcl_DeleteHashTable(&accum);
}

//-----------------------------------------------------------------------------
// Search the given image using the 30x30 candidate detector.  The levelmask
// is used to indicate which portions of the image to not bother searching,
// because a face has already been found there.  When a face is found,
// the callback function is called with information about the detection.

void
SearchUme (Image &Img, Image *pLevMask, int iLev,
           CALLBACK_FUN callbackFunc, ClientData callbackData)
{
int x, y, i, j;

if (pLevMask != NULL)
    {
    // For each 10x10 pixel block where the center of a face could be,
    // see if more than 5 pixel locations still need to be scanned.  If
    // so, the block must be scanned.  This is indicated by a flag placed
    // at the upper-left corner of each block.
    // In pLevMask: 0 means do scan, 255 means don't scan.

    int xp = (Img.width + 9) / 10;
    int yp = (Img.height + 9) / 10;
    for (y = 0; y < yp; y++)
        {
        for (x = 0; x < xp; x++)
            {
            int Total = 0, n = 0;
            for (j = y * 10; j < y * 10 + 10; j++)
                for (i = x * 10; i < x * 10 + 10; i++)
                    if (i < pLevMask->width && j < pLevMask->height)
                        {
                        Total++;
                        if (!(*pLevMask)(i, j))
                            n++;
                        }
            if (n <= 5)
                (*pLevMask)(x * 10,  y * 10) = 255;
            else
                (*pLevMask)(x * 10, y * 10) = 0;
            }
        }
    int TempImage[900];                         // used to store the window 30x30 = 900
    int Hist[256];
    for (y = 0; y < Img.height; y += 10)        // for each block
        {
        for (x = 0; x < Img.width; x += 10)
            {
            if (pLevMask != NULL && (*pLevMask)(x, y))
                continue;

            memset(Hist, 0, sizeof(Hist));

            // Copy the window from the image into TempImage.  The first loop is used
            // when the window is entirely inside the image, the second one is
            // used otherwise.  For pixels outside the image, the pixels at the
            // edge of the image are replicated.  The histogram is updated.

            int *pTo = TempImage;
            if (x >= 10 && y >= 10 && x + 20 <= Img.width && y + 20 <= Img.height)
                {
                for (j = y - 10; j < y + 20; j++)
                    for (i = x - 10; i < x + 20; i++)
                        {
                        int val = Img(i, j);
                        Hist[val]++;
                        (*(pTo++)) = val;
                        }
                }
            else
                {
                for (j = y - 10; j < y + 20; j++)
                    for (i = x - 10; i < x + 20; i++)
                        {
                        int ii = i;
                        if (ii < 0)
                            ii = 0;
                        if (ii >= Img.width)
                            ii = Img.width-1;
                        int jj = j;
                        if (jj<0)
                            jj = 0;
                        if (jj >= Img.height)
                            jj = Img.height - 1;
                        int val = Img(ii, jj);
                        Hist[val]++;
                        (*(pTo++)) = val;
                        }
                }

            // build a cumulative histogram

            int CumHist[256];
            pTo = CumHist;
            int *pFrom = Hist;
            int Total = 0;
            for (i = 0; i < 256; i++)
                {
                int old = Total;
                Total += *(pFrom++);
                *(pTo++) = old + Total;
                }
            // apply histogram equalization, write image into network input units

            const double Scale = 1.0 / Total;
            ForwardUnit *pUnit = &(gNetList[0]->pUnitList[1]);
            pFrom = TempImage; // 30x30 window
            for (i = 0; i < 900; i++)
                (pUnit++)->activation = (_FLOAT)(CumHist[*(pFrom++)] * Scale - 1.0);

            // Apply the network.
            // If there is a detection, call the callback function to record the detection.

            const double output = ForwardPass(gNetList[0]);
            if (output > 0)
                callbackFunc(callbackData, &Img, x-5, y-5, 20, 20, iLev,
                    POW12(iLev), output);
            }
        }
    }
}
