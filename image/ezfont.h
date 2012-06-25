/*----------------------
   ezfont.h header file
  ----------------------*/

#if _MSC_VER // microsoft, whole file

#if !defined(ezfont_h)

#include <windows.h>
#include <string.h>
#include <math.h>

#define ezfont_h
#ifdef __cplusplus
extern "C"
#endif

HFONT EzCreateFont(HDC hdc, char * szFaceName, int iDeciPtHeight, int iDeciPtWidth, int iAttributes, BOOL fLogRes);

#define EZ_ATTR_BOLD          1
#define EZ_ATTR_ITALIC        2
#define EZ_ATTR_UNDERLINE     4
#define EZ_ATTR_STRIKEOUT     8

#endif  /* ezfont_h */

#endif // _MSC_VER

