@rem tar-stasm.bat: tar all files needed for building stasm
@rem Do a "make veryclean" before running this
@rem The "if errorlevel 1" tests against any err level
@rem greater than or equal to 1

@set V=3.1

@rem make sure we are in the right directory
cd ..\..\stasm%V%\tools
                        @if errorlevel 1 goto error
cd ..\..
                        @if errorlevel 1 goto error
tar -cf stasm-src-%V%.tar stasm%V%
                        @if errorlevel 1 goto error
gzip --best stasm-src-%V%.tar
                        @if errorlevel 1 goto error
mv stasm-src-%V%.tar.gz /a/homepage/milbo.org/stasm-files
                        @if errorlevel 1 goto error
cp stasm%V%/HISTORY.txt /a/homepage/milbo.org/stasm-files
                        @if errorlevel 1 goto error
@exit /B 0
:error
@echo ==== ERROR ====
@exit /B 1
