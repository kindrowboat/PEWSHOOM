// $shape2r.cpp 3.0 milbo$ convert a shape file to a file readable by R

#include "stasm.hpp"

#define GET_WIDTH_FROM_IMAGE 0  // slower but more reliable, needed for varied.shape

//-----------------------------------------------------------------------------
static void PrintFaceDetParams (const char sDetTag[],
                                const vec_SHAPE  &Shapes,
                                const vec_string &Tags,
                                FILE *pFile)
{
unsigned iShape;
static unsigned iLast; // for efficiency, start off where we left last time

for (iShape = 0; iShape < Shapes.size(); iShape++)
    {
    if (0 == strcmp(sDetTag, Tags[iLast].c_str()))
        break;
    if (++iLast == Shapes.size())
        iLast = 0;
    }
if (iShape < Shapes.size()) // found it?
    {
    SHAPE Shape1 = Shapes[iLast];
    ASSERT(Shape1(2) == Shape1(3)); // face box width must equal shape box height
    fprintf(pFile, "%6.0f %6.0f %6.0f ", Shape1(0), Shape1(1), Shape1(2));
    ASSERT(Shape1.nelems() == 4 || Shape1.nelems() == 8);
    if (Shape1.nelems() == 8)       // eye positions available?
        fprintf(pFile, "%6.0f %6.0f %6.0f %6.0f ",
            Shape1(4), Shape1(5), Shape1(6), Shape1(7));
    else
        fprintf(pFile, "%6d %6d %6d %6d ", INVALID, INVALID, INVALID, INVALID);
    }
else                                // not found
    fprintf(pFile, "%6d %6d %6d %6d %6d %6d %6d ",
              INVALID, INVALID, INVALID, INVALID,
              INVALID, INVALID, INVALID);
}

//-----------------------------------------------------------------------------
int main (int argc, const char *argv[])
{
if (argc != 2)
    {
    printf("usage: shape2r file.shape\n");
    exit(-1);
    }
vec_SHAPE  Shapes;
vec_string Tags;

char sImageDirs[SLEN];
ReadShapeFile(Shapes, Tags, sImageDirs, NULL, 0, 0, argv[1]);

char sFile[SLEN]; sprintf(sFile, "_%s.R", sGetBase(argv[1]));   // R file name
lprintf("Writing %s\n", sFile);
FILE *pFile = Fopen(sFile, "w");

fprintf(pFile,
          "#  1            2   3      4      5      6      7      8      9     "
          "10     11     12     13    14      15     16     17     18     19     "
          "20     21     22     23     24     25     26     "
          "27    28      29     30     31     32     33\n");

fprintf(pFile,
          "file         type rev imagew imageh  meanx  meany  x.ext  y.ext    "
          "lex    ley    rex    rey   lmx     lmy    rmx    rmy   tipx   tipy   "
          "vj.x   vj.y   vj.w vj.lex vj.ley vj.rex vj.rey   "
          "ro.x   ro.y   ro.w ro.lex ro.ley ro.rex ro.rey\n");

const unsigned nShapes = Shapes.size();
unsigned iShape;
printf("Processing ");
for (iShape = 0; iShape < nShapes; iShape++)
    {
    if (iShape % (nShapes/10) == 0)
        {
        printf(".");
        fflush(stdout);
        }
    const char *sTag = Tags[iShape].c_str();
    if (sTag[0] == '0')
        {
        SHAPE *pShape = &Shapes[iShape];
        SHAPE Shape17;
        const char *sBase = sGetBasenameFromTag(sTag);
        ConvertToShape17(Shape17, *pShape);
        unsigned nrows = pShape->nrows();
        char type = tolower(sBase[0]);
        int w, h, itip; // width and height of image, index of tip of chin
        switch(type)
            {
            case 'a': ASSERT(nrows == 22);       w = 768; h = 576; itip = 19; break;
            case 'b': ASSERT(nrows == 20);       w = 384; h = 286; itip = 19; break;
            case 'm': ASSERT(nrows == 68 || nrows == 76 || nrows == 84);
                                                 w = 720; h = 576; itip = 7;  break;
            default:  w = 0; h = 0; itip = 0;
                      Warn("%s is an unrecognized file type "
                           "(w, h, tipx, tipx will be wrong)", sTag);
                      break;
            }
#if GET_WIDTH_FROM_IMAGE    // needed for varied.shape
        Image Img;
        sLoadImageGivenDirs(Img, sBase, sImageDirs, argv[1], true);
        w = Img.width;
        h = Img.height;
#endif
        fprintf(pFile,
            "%-12.12s    %c %d    %5d  %5d %6.1f %6.1f %6.1f %6.1f "
            "%6.1f %6.1f %6.1f %6.1f %6.1f %6.1f %6.1f %6.1f %6.1f %6.1f ",
            sBase,                                      // file
            type,                                       // type
            ((sBase[1] == 'r')? 1: 0),                  // 1 if image mirrored
            w,                                          // image width
            h,                                          // image height
            pShape->col(VX).sum() / nrows,              // meanx
            pShape->col(VY).sum() / nrows,              // meany
            xShapeExtent(*pShape),                      // x.ext
            yShapeExtent(*pShape),                      // y.ext
            Shape17(0, VX),                             // lex
            Shape17(0, VY),                             // ley
            Shape17(1, VX),                             // rex
            Shape17(1, VY),                             // rey
            Shape17(2, VX),                             // lmx
            Shape17(2, VY),                             // lmy
            Shape17(3, VX),                             // rmx
            Shape17(3, VY),                             // rmy
            (*pShape)(itip, VX),                        // tipx (tip of chin)
            (*pShape)(itip, VY));                       // tipy

        char s[SLEN];
        sprintf(s, "1000 %s", sBase);
        PrintFaceDetParams(s, Shapes, Tags, pFile);
        sprintf(s, "2000 %s", sBase);
        PrintFaceDetParams(s, Shapes, Tags, pFile);
        fprintf(pFile, "\n");
        }
    }
printf("\n");
return 0;       // return 0 on success
}
