// $stasm_opencv_example.cpp 3.0 milbo$ locate facial landmarks using the Stasm DLL
//
// To build, do something like (you may have to tweak the flags for your environment):
//
// cl -nologo -O2 -W3 -MT -EHsc -DWIN32 stasm_opencv_example.cpp \
//   -I%OPENCV_HOME%\include\opencv %OPENCV_HOME%\lib\cv210.lib \
//   %OPENCV_HOME%\lib\cxcore210.lib %OPENCV_HOME%\lib\highgui210.lib \
//   ..\data\stasm_dll.lib


#include <stdio.h>
#include <assert.h>
#if _MSC_VER==1200 // hack for Visual C 6.0 (standard OpenCV .h files cause compiler errors)
  #include "../stasm/stasm_cv.h"
#else
  #include "cv.h"
  #include "highgui.h"
#endif
#include "../stasm/stasm_dll.hpp"   // for AsmSearchDll

int main (void)
{
    const char *image_name = "../data/test-image.jpg";

    IplImage *img = cvLoadImage(image_name, CV_LOAD_IMAGE_COLOR);
    if(img == NULL) {
        printf("Error: Cannot open %s\n", image_name);
        return -1;
    }
    // sanity checks (AsmSearchDll assumes imageData is vector of b,r,g bytes)

    if(img->nChannels != 3 || img->depth != IPL_DEPTH_8U ||
            img->origin != 0 || img->widthStep != 3 * img->width) {
        printf("Error: %s is an unrecognized image type\n", image_name);
        return -1;
    }

    // locate the facial landmarks with stasm

    int nlandmarks;
    int landmarks[500]; // space for x,y coords of up to 250 landmarks
    AsmSearchDll(&nlandmarks, landmarks,
                 image_name, img->imageData, img->width, img->height,
                 1 /* is_color */, NULL /* conf_file0 */, NULL /* conf_file1 */);
    if (nlandmarks == 0) {
        printf("\nError: Cannot locate landmarks in %s\n", image_name);
        return -1;
    }

#if 0 // print the landmarks if you want
    printf("landmarks:\n");
    for (int i = 0; i < nlandmarks; i++)
        printf("%3d: %4d %4d\n", i, landmarks[2 * i], landmarks[2 * i + 1]);
#endif

    // draw the landmarks on the image

    assert(sizeof(int) == 4); // needed for CvPoint typecast below
    int *p = landmarks;
    cvPolyLine(img, (CvPoint **)&p, &nlandmarks, 1, 1, CV_RGB(255,0,0));

    // show the image with the landmarks

    cvShowImage("stasm example", img);
    cvWaitKey(0);
    cvDestroyWindow("stasm example");
    cvReleaseImage(&img);

    return 0;
}
