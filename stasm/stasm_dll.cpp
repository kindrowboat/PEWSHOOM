// stasm_dll.cpp

#include "windows.h"
#include "stasm.hpp"
#include "stasm_dll.hpp"

int WINAPI DllMain (HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}

void AsmSearchDll (
    int *pnlandmarks,          // out: number of landmarks, 0 if can't get landmarks
    int landmarks[],           // out: the landmarks, caller must allocate
    const char image_name[],   // in: used in internal error messages, if necessary
    const char image_data[],   // in: image data, 3 bytes per pixel if is_color
    const int width,           // in: the width of the image
    const int height,          // in: the height of the image
    const int is_color,        // in: 1 if RGB image, 0 if grayscale
    const char conf_file0[],   // in: 1st config filename, NULL for default
    const char conf_file1[])   // in: 2nd config filename, NULL for default, "" if none
{
    ASSERT(sizeof(int) == 4);  // sanity checks
    ASSERT(image_name[0]);
    ASSERT(width > 10 && height > 10);
    ASSERT(is_color == 0 || is_color == 1);

    char sConfFile0[SLEN];
    if(conf_file0 == NULL)
        strcpy(sConfFile0, "../data/mu-68-1d.conf");
    else if(conf_file0[0] == 0)
        Err("AsmSearchDll: conf_file0[0] == 0");
    else
        strcpy(sConfFile0, conf_file0);
    ASSERT(sConfFile0[0]);

    char sConfFile1[SLEN];
    if(conf_file1 == NULL)
        strcpy(sConfFile1, "../data/mu-76-2d.conf");
    else if(conf_file1[0] == 0)
        sConfFile1[0] = 0;
    else
        strcpy(sConfFile1, conf_file1);

    RgbImage Img(width, height);

    // copy image_data into Img, flipping row order

    if(is_color)
        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++)
                {
                int i1 = i * width + j;
                int i2 = 3 * ((height - i - 1) * width + j);
                Img.buf[i1].Blue  = image_data[i2];
                Img.buf[i1].Green = image_data[i2+1];
                Img.buf[i1].Red   = image_data[i2+2];
                }
    else
        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++)
                {
                int i1 = i * width + j;
                Img.buf[i1].Blue = Img.buf[i1].Green = Img.buf[i1].Red =
                    image_data[(height - i - 1) * width + j];
                }

    SHAPE StartShape;           // dummy args
    DET_PARAMS DetParams;       // dummy arg for AsmSearch
    double MeanTime;            // dummy arg for AsmSearch
    SHAPE Shape = AsmSearch(StartShape, DetParams, MeanTime,
                            Img, image_name, FALSE, sConfFile0, sConfFile1);

    *pnlandmarks = 0;
    if (Shape.nrows())          // successfully located landmarks?
        {
        // convert from stasm coords to OpenCV coords

        Shape.col(VX) += Img.width / 2;
        Shape.col(VY) = Img.height / 2 - Shape.col(VY);

        *pnlandmarks = Shape.nrows();
        for (int iPoint = 0; iPoint < *pnlandmarks; iPoint++)
            {
            landmarks[2 * iPoint]     = iround(Shape(iPoint, VX));
            landmarks[2 * iPoint + 1] = iround(Shape(iPoint, VY));
            }
        }
}
