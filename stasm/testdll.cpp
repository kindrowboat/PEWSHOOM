// $testdll.cpp 3.0 milbo$ program to test AsmSearchDll

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>
#if _MSC_VER==1200 // hack for Visual C 6.0 (standard OpenCV .h files cause compiler errors)
  #include "../stasm/stasm_cv.h"
#else
  #include "cv.h"
  #include "highgui.h"
#endif
#include "../stasm/stasm_dll.hpp"

int main (int argc, char *argv[])
{
    if (argc != 4 && argc != 5) {
        printf("Usage: testdll image is_color conf_file0 [conf_file1]\n"
               "Example: testdll ../data/test-image.jpg 1 NULL NULL\n");
        return -1;
    }
    const char *image_name = argv[1];
    assert(argv[2][0] == '0' || argv[2][0] == '1');
    const bool is_color = argv[2][0] == '1';
    char *conf_file0 = argv[3];
    char *conf_file1 = (argc == 4)? "": argv[4];

    IplImage *img;
    if(is_color)
        img = cvLoadImage(image_name, CV_LOAD_IMAGE_COLOR);
    else
        img = cvLoadImage(image_name, CV_LOAD_IMAGE_GRAYSCALE);
    if(img == NULL) {
        printf("Error: Cannot open %s\n", image_name);
        return -1;
    }

    int nlandmarks;
    int landmarks[500]; // space for x,y coords of up to 250 landmarks
    AsmSearchDll(&nlandmarks, landmarks,
                 image_name, img->imageData, img->width, img->height,
                 is_color,
                 strcmp(conf_file0, "NULL") == 0? NULL: conf_file0,
                 strcmp(conf_file1, "NULL") == 0? NULL: conf_file1);

    if (nlandmarks == 0) {
        printf("\nError: Cannot locate landmarks in %s\n", image_name);
        return -1;
    }
    cvReleaseImage(&img);

    printf("%s landmarks:\n", image_name);
    for (int i = 0; i < nlandmarks; i++)
        printf("%3d: %4d %4d\n", i, landmarks[2 * i], landmarks[2 * i + 1]);

    return 0;
}
