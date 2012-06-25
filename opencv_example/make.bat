@rem make.bat: build and run stasm_opencv_example.exe

set OPENCV_HOME=C:\OpenCV2.1

@rem Ensure we are in the correct directory

cd ..
                                        @if errorlevel == 1 goto error
cd opencv_example
                                        @if errorlevel == 1 goto error

@rem Copy OpenCV DLLs to the current directory to avoid DLL problems
@rem May not be necessary, depending on your environment

copy ..\data\stasm_dll.dll .
                                        @if errorlevel == 1 goto error
copy %OPENCV_HOME%\bin\cv210.dll
                                        @if errorlevel == 1 goto error
copy %OPENCV_HOME%\bin\cxcore210.dll
                                        @if errorlevel == 1 goto error
copy %OPENCV_HOME%\bin\highgui210.dll .
                                        @if errorlevel == 1 goto error

@rem The cl flags here may not be optimum, but seem ok

cl -nologo -O2 -W3 -MT -EHsc -DWIN32 stasm_opencv_example.cpp -I%OPENCV_HOME%\include\opencv %OPENCV_HOME%\lib\cv210.lib %OPENCV_HOME%\lib\cxcore210.lib %OPENCV_HOME%\lib\highgui210.lib ..\data\stasm_dll.lib
                                        @if errorlevel == 1 goto error

stasm_opencv_example
                                        @if errorlevel == 1 goto error
@goto done
:error
@echo ==== ERROR ====
:done
