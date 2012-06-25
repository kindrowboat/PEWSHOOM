@set OLDDIR=D:/a/stasm/stasm
@set NEWDIR=/a/stasm/stasm

@diff %OLDDIR%/iland/*.[ch]*            %NEWDIR%/iland
@diff %OLDDIR%/image/*.[ch]*            %NEWDIR%/image
@diff %OLDDIR%/jpeg/*.[ch]*             %NEWDIR%/jpeg
@diff %OLDDIR%/marki/*.[chr]*           %NEWDIR%/marki
@diff %OLDDIR%/mat/*.[ch]*              %NEWDIR%/mat
@diff %OLDDIR%/mdiff/*.[ch]*            %NEWDIR%/mdiff
@diff %OLDDIR%/minimal/*.[ch]*          %NEWDIR%/minimal
@diff %OLDDIR%/regex/*.[ch]*            %NEWDIR%/regex
@diff %OLDDIR%/rowley/*.[ch]*           %NEWDIR%/rowley
@diff %OLDDIR%/stasm/*.[ch]*            %NEWDIR%/stasm
@diff %OLDDIR%/tasm-example/*.[ch]*     %NEWDIR%/tasm-example
@diff %OLDDIR%/tasm/*.[ch]*             %NEWDIR%/tasm
@diff %OLDDIR%/tools/*.[abchlnpstwx]*   %NEWDIR%/tools
@diff %OLDDIR%/wasm/*.[ch]*             %NEWDIR%/wasm

@diff %OLDDIR%/linux/*mak*              %NEWDIR%/linux
@diff %OLDDIR%/mingw/*mak*              %NEWDIR%/mingw
@diff %OLDDIR%/vc9/*mak*                %NEWDIR%/vc9
@diff %OLDDIR%/vc6/*mak*                %NEWDIR%/vc6
@rem @diff %OLDDIR%/vc10/*mak*               %NEWDIR%/vc10

@diff %OLDDIR%/data/*.[achlnswx]*       %NEWDIR%/data
@diff %OLDDIR%/tests/*.[achlnswx]*      %NEWDIR%/tests

@diff D:/a/stasm/installers/WinThatch-installer/* /a/stasm/installers/WinThatch-installer
@diff D:/a/stasm/installers/casm-installer/*      /a/stasm/installers/casm-installer
@diff D:/a/stasm/installers/iland-installer/*     /a/stasm/installers/iland-installer
@diff D:/a/stasm/installers/wasm-installer/*      /a/stasm/installers/wasm-installer
