@rem The following is a basic check that you have VC 9.0 (2008) on your path

@..\tools\which cl | ..\tools\grep "Visual Studio 9.0" >NUL && goto doit
@echo Environment is not VC 9.0
@goto end
:doit

@nmake -nologo CFG=Release -f ../vc10/makefile %1 %2 %3
@rem @nmake -nologo CFG=Debug -f ../vc10/makefile %1 %2 %3
@rem TODO following has not been tested
@rem @nmake -nologo CFG=Profile -f ../vc10/makefile %1 %2 %3

:end
