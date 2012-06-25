// $marki.hpp 3.0 milbo$

#if !defined(marki_hpp)
#define marki_hpp

//-----------------------------------------------------------------------------
// resource definitions for Toolbar

#define BUTTON_Standard     TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0
#define BUTTON_Separator    TBSTATE_ENABLED,TBSTYLE_SEP,0,0,0,0

#define IDR_MARKI_BUTTONS   333     // id of toolbar bitmap

#define IDM_FirstInToolbar  100

#define IDM_Save            100
#define IDM_Lock            101
#define IDM_Undo            102
#define IDM_Redo            103
#define IDM_Equalize        104
#define IDM_ConnectDots     105
#define IDM_ShowNbrs        106
#define IDM_ShowCurrentNbr  107
#define IDM_HideLandmarks   108
#define IDM_WriteImage      109
#define IDM_Crop            110
#define IDM_ZoomOut         111
#define IDM_ZoomIn          112
#define IDM_AutoNextImage   113
#define IDM_PrevImage       114
#define IDM_NextImage       115
#define IDM_AutoNextPoint   116
#define IDM_PrevPoint       117
#define IDM_NextPoint       118
#define IDM_JumpToLandmark  119
#define IDM_Help            120
#define IDM_Blank           121

// for dialog window
#define IDC_STATIC              -1
#define IDC_LANDMARK            1000
#define IDC_IMAGE_NBR           1001
#define IDC_FILE_NAME           1002
#define IDC_SAVED               1003
#define IDC_MARKED              1004
#define IDC_IMAGE_STRING        1005
#define IDC_NBR_IMAGES          1006
#define IDC_NBR_LANDMARKS       1007

#endif // marki_hpp
