@rem make and test all stasm builds under Windows
@rem TODO add MSBuild for vc10

@rem @set CLEAN=
@set CLEAN=clean

@rem @set TEST=
@set TEST=test
@rem @set TEST=fulltest

@set STASM_DIR=\a\stasm\stasm
@set INSTALL_DIR=\a\stasm\installers
@set NM=nmake -nologo -f ../vc10/makefile

@rem make sure we are in the right directory
cd %STASM_DIR%\tools
                                @if errorlevel 1 goto error
@rem ======================================================
cd ..\vc6
                                @if errorlevel 1 goto error
@rem set environment variables for VC6
call vc6
                                @if errorlevel 1 goto error
%NM% CFG=Debug   %CLEAN%
                                @if errorlevel 1 goto error
@%NM% CFG=Debug
                                @if errorlevel 1 goto error
@%NM% CFG=Debug   %TEST%
                                @if errorlevel 1 goto error
%NM% CFG=Release %CLEAN%
                                @if errorlevel 1 goto error
@%NM% CFG=Release
                                @if errorlevel 1 goto error
@%NM% CFG=Release %TEST%
                                @if errorlevel 1 goto error
@rem ======================================================
cd ..\vc9
                                @if errorlevel 1 goto error
call vc9
                                @if errorlevel 1 goto error
%NM% CFG=Debug %CLEAN%
                                @if errorlevel 1 goto error
@%NM% CFG=Debug
                                @if errorlevel 1 goto error
@%NM% CFG=Debug   %TEST%
                                @if errorlevel 1 goto error
%NM% CFG=Release %CLEAN%
                                @if errorlevel 1 goto error
@%NM% CFG=Release
                                @if errorlevel 1 goto error
@%NM% CFG=Release %TEST%
                                @if errorlevel 1 goto error
======================================================
@cd ..\vc9
                                      @if errorlevel 1 goto error
@call vc9
                                      @if errorlevel 1 goto error
%NM% CFG=Release %CLEAN%
                                      @if errorlevel 1 goto error
@set MSBUILD_FLAGS=/nologo /verbosity:minimal /property:Configuration=Release
                                      @if errorlevel 1 goto error
MSBuild casm.sln      %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild marki.sln     %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild minimal.sln   %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild stasm.sln     %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild tasm.sln      %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild wasm.sln      %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild thatch.sln    %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild winthatch.sln %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
@rem TODO revisit, this keeps make test happy
@copy ..\vc6\fdet.exe .
                                      @if errorlevel 1 goto error
@copy ..\vc6\testdll.exe .
                                      @if errorlevel 1 goto error
@copy ..\data\stasm_dll.dll .
                                      @if errorlevel 1 goto error
@mv Release\*.exe .
                                      @if errorlevel 1 goto error
@%NM% CFG=Release %TEST%
                                      @if errorlevel 1 goto error
%NM% CFG=Debug %CLEAN%
                                      @if errorlevel 1 goto error

@set MSBUILD_FLAGS=/nologo /verbosity:minimal /property:Configuration=Debug
                                      @if errorlevel 1 goto error
MSBuild casm.sln      %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild marki.sln     %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild minimal.sln   %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild stasm.sln     %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild tasm.sln      %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild wasm.sln      %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild thatch.sln    %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild winthatch.sln %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
@copy ..\vc6\fdet.exe .
                                      @if errorlevel 1 goto error
@copy ..\vc6\testdll.exe .
                                      @if errorlevel 1 goto error
@copy ..\data\stasm_dll.dll .
                                      @if errorlevel 1 goto error
mv Debug\*.exe .
@%NM% CFG=Debug %TEST%
                                      @if errorlevel 1 goto error
@rem ======================================================
cd ..\vc10
                                @if errorlevel 1 goto error
call vc10
                                @if errorlevel 1 goto error
%NM% CFG=Release %CLEAN%
                                @if errorlevel 1 goto error
@%NM% CFG=Release
                                @if errorlevel 1 goto error
@%NM% CFG=Release %TEST%
                                @if errorlevel 1 goto error
%NM% CFG=Debug %CLEAN%
                                @if errorlevel 1 goto error
@%NM% CFG=Debug
                                @if errorlevel 1 goto error
@copy ..\vc6\fdet.exe .
                                      @if errorlevel 1 goto error
@copy ..\vc6\testdll.exe .
                                @if errorlevel 1 goto error
@copy ..\data\stasm_dll.dll .
                                      @if errorlevel 1 goto error
@%NM% CFG=Debug   %TEST%
                                @if errorlevel 1 goto error
@rem ======================================================
@cd ..\vc10
                                      @if errorlevel 1 goto error
@call vc10
                                      @if errorlevel 1 goto error
%NM% CFG=Release %CLEAN%
                                      @if errorlevel 1 goto error
@set MSBUILD_FLAGS=/nologo /verbosity:minimal /property:Configuration=Release
                                      @if errorlevel 1 goto error
MSBuild casm.sln      %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild marki.sln     %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild minimal.sln   %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild stasm.sln     %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild tasm.sln      %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild wasm.sln      %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
@rem TODO build fails here --- but not from IDE?
@rem MSBuild thatch.sln    %MSBUILD_FLAGS%
@rem                                   @if errorlevel 1 goto error
@rem MSBuild winthatch.sln %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
@mv Release\*.exe .
                                      @if errorlevel 1 goto error
@copy ..\vc6\fdet.exe .
                                      @if errorlevel 1 goto error
@copy ..\vc6\testdll.exe .
                                      @if errorlevel 1 goto error
@copy ..\data\stasm_dll.dll .
                                      @if errorlevel 1 goto error
@%NM% CFG=Release %TEST%
                                      @if errorlevel 1 goto error
%NM% CFG=Debug %CLEAN%
                                      @if errorlevel 1 goto error

@set MSBUILD_FLAGS=/nologo /verbosity:minimal /property:Configuration=Debug
                                      @if errorlevel 1 goto error
MSBuild casm.sln      %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild marki.sln     %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild minimal.sln   %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild stasm.sln     %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild tasm.sln      %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
MSBuild wasm.sln      %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
@rem TODO build fails here --- but not from IDE?
@rem MSBuild thatch.sln    %MSBUILD_FLAGS%
@rem                                   @if errorlevel 1 goto error
@rem MSBuild winthatch.sln %MSBUILD_FLAGS%
                                      @if errorlevel 1 goto error
@copy ..\vc6\fdet.exe .
                                      @if errorlevel 1 goto error
@copy ..\vc6\testdll.exe .
                                      @if errorlevel 1 goto error
@copy ..\data\stasm_dll.dll .
                                      @if errorlevel 1 goto error
mv Debug\*.exe .
@%NM% CFG=Debug %TEST%
                                      @if errorlevel 1 goto error
@rem ======================================================
cd ..\mingw
                                @if errorlevel 1 goto error
call mingw
                                @if errorlevel 1 goto error
@make %CLEAN%
                                @if errorlevel 1 goto error
@make
                                @if errorlevel 1 goto error
@make test
                                @if errorlevel 1 goto error
@rem ======================================================

@cd %INSTALL_DIR%\casm
                                @if errorlevel 1 goto error
@call make
                                @if errorlevel 1 goto error
@cd %INSTALL_DIR%\iland
                                @if errorlevel 1 goto error
@call make
                                @if errorlevel 1 goto error
@cd %INSTALL_DIR%\WinThatch
                                @if errorlevel 1 goto error
@call make
                                @if errorlevel 1 goto error
cd %STASM_DIR%\tools
                                @if errorlevel 1 goto error
@exit /B 0
:error
@echo ==== ERROR ====
@exit /B 1
