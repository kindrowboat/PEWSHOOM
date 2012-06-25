@rem Batch file for profiling marki
@rem First do a make clean and a profiling build: nmake -nologo CFG=Profile -f makefile

prep -nologo -om -ft marki
@rem use -at for attribution data (i.e. which funcs called which funcs)
@rem prep -nologo -om -ft -at marki
@if errorlevel == 1 goto error 

profile -nologo marki -S -B /a1/faces/put/tools/put.shape
@if errorlevel == 1 goto error 

prep -nologo -m marki
@if errorlevel == 1 goto error 

@rem if you list prep -at then you must use -flat here else plist crashes
plist -nologo -flat marki >marki.profileResults
@if errorlevel == 1 goto error 

@exit /B 0
:error
@echo PROFILING ERROR
@exit /B 1
