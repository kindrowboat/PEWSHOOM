// $imgiven.hpp 3.0 milbo$ routines for finding images given directories

#if !defined(imgiven_hpp)
#define imgiven_hpp

const char *
sGetPathGivenDirs(
        char sPath[],               // out
        const char sBase[],         // in
        const char sDirs[],         // in: filename base (dir and ext will be ignored)
        const char sShapeFile[],    // in: shape file holding sDirs, for err msgs
        bool fExitOnErr=true);      // in

const char *
sLoadImageGivenDirs(
        Image &Img,                 // out
        const char sFile[],         // in: only base name of this is used
        const char sDirs[],         // in
        const char sShapeFile[],    // in: shape file holding sDirs, for err msgs
        bool fExitOnErr=true);      // in

const char *
sLoadImageGivenDirs(
        RgbImage &Img,              // out
        const char sFile[],         // in: only base name of this is used
        const char sDirs[],         // in
        const char sShapeFile[],    // in: shape file holding sDirs, for err msgs
        bool fExitOnErr=true);      // in

#endif // imgiven_hpp
