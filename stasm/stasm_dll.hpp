// stasm_dll.hpp

#ifndef stasm_dll_hpp
#define stasm_dll_hpp

extern "C" __declspec(dllexport)
void AsmSearchDll(
    int *pnlandmarks,          // out: number of landmarks, 0 if can't get landmarks
    int landmarks[],           // out: the landmarks, caller must allocate
    const char image_name[],   // in: used in internal error messages, if necessary
    const char image_data[],   // in: image data, 3 bytes per pixel if is_color
    const int width,           // in: the width of the image
    const int height,          // in: the height of the image
    const int is_color,        // in: 1 if RGB image, 0 for grayscale
    const char conf_file0[],   // in: 1st config filename, NULL for default
    const char conf_file1[]);  // in: 2nd config filename, NULL for default, "" if none

#endif // stasm_dll_hpp
