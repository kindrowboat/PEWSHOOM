# Microsoft Developer Studio Project File - Name="stasm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=stasm - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "stasm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "stasm.mak" CFG="stasm - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "stasm - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "stasm - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "stasm - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "C:\OpenCV2.1\include\opencv" /I "..\regex" /I "..\gsl" /I "..\gsl\gsl" /I "..\image" /I "..\jpeg" /I "..\mat" /I "..\rowley" /I "..\stasm" /I "..\tasm" /I "..\png" /I "..\png\pnglib" /I "..\png\zlib" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "HAVE_CONFIG_H" /D "STDC_HEADERS" /D "REGEX_MALLOC" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\gsl\lib_ms\mini_gslML.lib ..\gsl\lib_ms\gslcblasML.lib ..\jpeg\lib_ms\libjpeg.lib ..\png\pnglib\libpng.lib ..\png\zlib\zlib.lib C:\OpenCV2.1\lib\cv210.lib C:\OpenCV2.1\lib\cxcore210.lib C:\OpenCV2.1\lib\highgui210.lib setargv.obj /nologo /subsystem:console /machine:I386 /nodefaultlib:"LIBC"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "stasm - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /I "." /I "C:\OpenCV2.1\include\opencv" /I "..\regex" /I "..\gsl" /I "..\gsl\gsl" /I "..\image" /I "..\jpeg" /I "..\mat" /I "..\rowley" /I "..\stasm" /I "..\tasm" /I "..\png" /I "..\png\pnglib" /I "..\png\zlib" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "HAVE_CONFIG_H" /D "STDC_HEADERS" /D "REGEX_MALLOC" /YX /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\gsl\lib_ms\mini_gslML.lib ..\gsl\lib_ms\gslcblasML.lib ..\jpeg\lib_ms\libjpeg.lib ..\png\pnglib\libpng.lib ..\png\zlib\zlib.lib C:\OpenCV2.1\lib\cv210.lib C:\OpenCV2.1\lib\cxcore210.lib C:\OpenCV2.1\lib\highgui210.lib setargv.obj /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"LIBCD" /nodefaultlib:"LIBC" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "stasm - Win32 Release"
# Name "stasm - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\stasm\asmsearch.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\atface.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\err.cpp
# End Source File
# Begin Source File

SOURCE=..\image\ezfont.c
# End Source File
# Begin Source File

SOURCE=..\rowley\find.cpp
# End Source File
# Begin Source File

SOURCE=..\rowley\follow.cpp
# End Source File
# Begin Source File

SOURCE=..\rowley\forward.cpp
# End Source File
# Begin Source File

SOURCE=..\image\imequalize.cpp
# End Source File
# Begin Source File

SOURCE=..\image\imfile.cpp
# End Source File
# Begin Source File

SOURCE=..\image\imgiven.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\imshape.cpp
# End Source File
# Begin Source File

SOURCE=..\image\imutil.cpp
# End Source File
# Begin Source File

SOURCE=..\image\imwrite.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\initasm.cpp
# End Source File
# Begin Source File

SOURCE=..\rowley\initnet.cpp
# End Source File
# Begin Source File

SOURCE=..\image\jpegutil.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\landmarks.cpp
# End Source File
# Begin Source File

SOURCE=..\rowley\list.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\main.cpp
# End Source File
# Begin Source File

SOURCE=..\mat\mat.cpp
# End Source File
# Begin Source File

SOURCE=..\mat\matvec.cpp
# End Source File
# Begin Source File

SOURCE=..\mat\mchol.cpp
# End Source File
# Begin Source File

SOURCE=..\tasm\mrand.cpp
# End Source File
# Begin Source File

SOURCE=..\image\pngutil.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\prof.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\readasm.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\readconf.cpp
# End Source File
# Begin Source File

SOURCE=..\png\readpng.c
# End Source File
# Begin Source File

SOURCE=..\regex\regex.c
# End Source File
# Begin Source File

SOURCE=..\stasm\release.cpp
# End Source File
# Begin Source File

SOURCE=..\image\rgbimutil.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\rowley.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\rowleyhand.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\safe_alloc.cpp
# End Source File
# Begin Source File

SOURCE=..\rowley\search.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\shapefile.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\shapemodel.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\sparsemat.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\startshape.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\stasm.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\tab.cpp
# End Source File
# Begin Source File

SOURCE=..\rowley\tclHash.c
# End Source File
# Begin Source File

SOURCE=..\stasm\util.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\violajones.cpp
# End Source File
# Begin Source File

SOURCE=..\stasm\vjhand.cpp
# End Source File
# Begin Source File

SOURCE=..\jpeg\wrbmp.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\jpeg\cderror.h
# End Source File
# Begin Source File

SOURCE=..\jpeg\cdjpeg.h
# End Source File
# Begin Source File

SOURCE=..\image\ezfont.h
# End Source File
# Begin Source File

SOURCE=..\jpeg\jconfig.h
# End Source File
# Begin Source File

SOURCE=..\jpeg\jerror.h
# End Source File
# Begin Source File

SOURCE=..\jpeg\jinclude.h
# End Source File
# Begin Source File

SOURCE=..\jpeg\jmorecfg.h
# End Source File
# Begin Source File

SOURCE=..\jpeg\jpeglib.h
# End Source File
# Begin Source File

SOURCE=..\regex\regex.h
# End Source File
# Begin Source File

SOURCE=..\regex\regex_config.h
# End Source File
# Begin Source File

SOURCE=..\rowley\tclHash.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
