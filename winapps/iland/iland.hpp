// $iland.hpp 3.0 milbo$

#if !defined(iland_hpp)
#define iland_hpp

//-----------------------------------------------------------------------------
// resource definitions for Toolbar

#define BUTTON_Standard     TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0
#define BUTTON_Separator    TBSTATE_ENABLED,TBSTYLE_SEP,0,0,0,0

#define IDR_ILAND_BUTTONS   333     // id of toolbar bitmap

#define IDM_FirstInToolbar  100

#define IDM_Open            100
#define IDM_ConnectDots     101
#define IDM_ShowNbrs        102
#define IDM_HideLandmarks   103
#define IDM_ZoomOut         104
#define IDM_ZoomIn          105
#define IDM_Help            106
#define IDM_Blank           107

// help window
#define IDC_STATIC            -1
#define IDC_VERSION         1001
#define IDC_STASM_VERSION   1002

// dialog window
#define IDC_TEXT            1010
#define IDC_BACK            100
#define IDC_NEXT            101

#endif // iland_hpp
