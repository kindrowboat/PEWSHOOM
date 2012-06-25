@rem make-clean.bat: clean up open_cv_example directory

@rem Ensure we are in the correct directory

cd ..
                                        @if errorlevel == 1 goto error
cd opencv_example
                                        @if errorlevel == 1 goto error
..\tools\rm -f *.dll *.exe *.obj

@goto done
:error
@echo ==== ERROR ====
:done
