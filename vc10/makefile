# $vc10\makefile 3.0 milbo$ makefile for stasm using Visual C++
#
# Release build: mkdir Release; nmake -nologo CFG=Release -f makefile
# Debug build:   mkdir Debug;   nmake -nologo CFG=Debug   -f makefile
#
# milbo durban aug07

!IF "$(CFG)" != "Release" && "$(CFG)" != "Debug" && "$(CFG)" != "Profile"
!ERROR Invalid CFG "$(CFG)"
!ENDIF

all: mdiff.exe stasm.exe tasm.exe minimal.exe marki.exe wasm.exe\
     casm.exe fdet.exe shape2r.exe thatch.exe iland.exe winthatch.exe\
     ../data/stasm_dll.dll testdll.exe stasm_opencv_example.exe 

# the following images are used only for extensive tests, you don't have to have them
BIOID_IMAGES= /a1/faces/bioid/lpgm/B*.pgm
AR_IMAGES=    /a1/faces/ar/pgm/a*.pgm
XM2VTS_IMAGES=/a1/faces/m2/pgm/m*.pgm
MUCT_IMAGES=  /a1/faces/muct/pgm/i*.pgm
MIRROR_IMAGES=/a1/faces/mirror/pgm/*.pgm

ALL_IMAGES=$(BIOID_IMAGES) $(XM2VTS_IMAGES) $(AR_IMAGES) $(MUCT_IMAGES)

REGEX_DEFS=-DHAVE_CONFIG_H -DSTDC_HEADERS -DREGEX_MALLOC

WINLIBS=advapi32.lib comctl32.lib comdlg32.lib gdi32.lib shell32.lib user32.lib vfw32.lib winmm.lib

OPENCV_HOME=C:\OpenCV2.1

OPENCV_DLLS=cv210.dll cxcore210.dll highgui210.dll

INCL=\
    -I$(OPENCV_HOME)/include/opencv\
    -I../regex\
    -I../gsl\
    -I../gsl/gsl\
    -I../image\
    -I../jpeg\
    -I../png\
    -I../png/pnglib\
    -I../png/zlib\
    -I../mat\
    -I../rowley\
    -I../stasm\
    -I../tasm

# following include allows this makefile to work for all supported versions of VisualC
!INCLUDE <./makefile-include>

clean:
	..\tools\rm -f Release/*.* Debug/*.* 
	..\tools\rm -f *.exe *._xe *.obj *.map *.opt *.pdb *.ilk *.ps th*.bmp
	..\tools\rm -f *.pbi *.pbo *.pbt *.pch *.opt *.ncb *.plg *.sdf
	..\tools\rm -f stasm_dll.*
	..\tools\rm -f ../opencv_example/*.dll ../opencv_example/*.exe ../opencv_example/*.obj

veryclean: clean
	..\tools\rm -f *.asm *.bmp *.conf *.tab *.profileResults *.XML
	..\tools\rm -f *.log ../winapps/marki/*.log ../winapps/wasm/*.log
	..\tools\rm -f Wasm-log.txt out/fdet-*.bmp search-results.bmp
	..\tools\rm -f ../*/default.tag *.tar *.gz _*.shape *.dll _*.R
	..\tools\rm -f *.vcproj.* *.vcxproj.user *.suo *.cache tab/* stasm_lib.dll

# generate face detector parameters for manual insertion into .shape file

vj-det-params:
	@echo === vj-det-params ===============================================
	fdet $(ALL_IMAGES)

rowley-det-params:
	@echo === rowley-det-params ===========================================
	fdet -r $(ALL_IMAGES)

# If any test below fails the make will abort with an error message

test-stasm: # stasm test with default config files and three images
	@echo === test-stasm ==================================================
	@echo === expect a warning for no-face.jpg
	stasm -iS ../tests/B1138_20.pgm ../tests/no-face.jpg ../data/test-image.jpg
	@echo The following mdiffs and cmp should give no output:
	..\tools\mdiff stasm.log ../tests/test-stasm.log
	cmp 1-B1138_20.bmp ../tests/1-B1138_20.bmp
	cmp s-B1138_20.bmp ../tests/s-B1138_20.bmp
	..\tools\rm -f stasm.log *-B1138_20.bmp s-no-face-noface.bmp 1-test-image.bmp s-test-image.bmp

test-stasm-with-shapefile: # stasm test with reference shapefile
	@echo === test-stasm-with-shapefile ===================================
 	stasm -if -t test.tab ../data/bioid.shape 0 -iS -p "B1138"
	@echo The following mdiff should give no output:
	..\tools\mdiff stasm.log ../tests/stasm-test-with-shapefile.log
	..\tools\mdiff test.tab ../tests/stasm-test-with-shapefile.tab
	..\tools\rm -f stasm.log test.tab *-B1138_20.bmp

test-minimal:
	@echo === test-minimal ================================================
	minimal
	@echo The following cmp should give no output:
	cmp search-results.bmp ../tests/minimal-search-results.bmp
	..\tools\rm -f search-results.bmp

test-dll:
	@echo === test-dll ====================================================
	testdll ../data/test-image.jpg 0 NULL NULL            >  test-dll.log
	-testdll ../tests/no-face.jpg 0 NULL NULL             >> test-dll.log
	testdll ../tests/B1138_20.pgm 1 NULL                  >> test-dll.log
	testdll ../data/test-image.jpg 1 NULL                 >> test-dll.log
	testdll ../data/test-image.jpg 1 ../data/model-1.conf >> test-dll.log
	testdll ../data/test-image.jpg 0 ../data/model-1.conf ../data/model-2.conf >> test-dll.log
	..\tools\mdiff test-dll.log ../tests/test-dll.log

test-stasm-row: # stasm rowley test with default config files
	@echo === test-stasm-row ==============================================
	stasm -r -iS ../data/test-image.jpg
	@echo The following mdiff and cmps should give no output:
	..\tools\mdiff stasm.log ../tests/test-row-stasm.log
	cmp 1-row-test-image.bmp ../tests/1-row-test-image.bmp
	cmp s-row-test-image.bmp ../tests/s-row-test-image.bmp
	..\tools\rm -f stasm.log *-test-image.bmp

# The following is an example of how to use tasm on non XM2VTS shapes
# It is also a good test of the tasm and stasm software
#
# TODO the sleep calls were added as a work around for the 
# "fopen: Permission denied" bug when tasm reads lot of JPEGs.

test-tasm-example:
	@echo === test-tasm-example ===========================================
	..\tools\sleep 5
	tasm -o example.asm ../tasm-example/tasm-example.conf
	@echo The following mdiffs should give no output:
	..\tools\mdiff tasm.log ../tasm-example/tests/tasm-example.log
	..\tools\mdiff example.asm ../tasm-example/tests/example.asm
	@echo === stasm test with above example asm file =====================
	stasm -iS -c ../tasm-example/example.conf ../data/test-image.jpg
	@echo The following mdiff and cmps should give no output:
	..\tools\mdiff stasm.log ../tasm-example/tests/vj-stasm.log
	cmp 1-test-image.bmp ../tasm-example/tests/1-test-image.bmp
	cmp s-test-image.bmp ../tasm-example/tests/s-test-image.bmp
	..\tools\rm -f tasm.log stasm.log example.asm *-test-image.bmp

test-2d: # tasm and stasm test with 2D profiles
	@echo === test-2d =====================================================
	..\tools\sleep 5
	tasm -o test-2d.asm ../tests/tasm-test20-2d.conf
	@echo The following mdiffs should give no output:
	..\tools\mdiff tasm.log ../tests/tasm-2d.log
	..\tools\mdiff test-2d.asm ../tests/test20-2d.asm
	@echo === stasm test with above asm file ==============================
	stasm -iS -c ../tests/test20-2d.conf ../data/test-image.jpg
	@echo The following mdiff and cmp should give no output:
	..\tools\mdiff stasm.log ../tests/stasm-2d.log
	cmp 1-test-image.bmp ../tests/test20-1-2d-test-image.bmp
	cmp s-test-image.bmp ../tests/test20-s-2d-test-image.bmp
	..\tools\rm -f tasm.log stasm.log test-2d.asm *-test-image.bmp

test-varied: # test with XM2VTS shapes and varied params
	@echo === test-varied =================================================
	tasm -o test-xm2vts-varied.asm ../tests/tasm-test20-xm2vts-varied.conf
	@echo The following mdiffs should give no output:
	..\tools\mdiff tasm.log ../tests/test-xm2vts-varied.log
	..\tools\mdiff test-xm2vts-varied.asm ../tests
	@echo === stasm test with above asm file ==============================
	stasm -c ../tests/test20-xm2vts-varied.conf ../data/test-image.jpg
	@echo The following mdiff and cmp should give no output:
	..\tools\mdiff stasm.log ../tests/stasm-xm2vts-varied.log
	cmp s-test-image.bmp  ../tests/s-xm2vts-varied-test-image.bmp
	..\tools\rm -f tasm.log stasm.log test-xm2vts-varied.asm *-test-image.bmp

test-fdet:
	@echo === test-fdet ===================================================
	fdet ../data/test-image.jpg
	@echo The following mdiffs and cmp should give no output:
	..\tools\mdiff fdet.log ../tests/test-fdet.log
	..\tools\rm -f fdet.log

test-mdiff:
	@echo === test-mdiff ==================================================
	@rem the - prefix means ignore correct "error" returns for non matches
	.\mdiff ../mdiff/tests/test1 ../mdiff/tests/test1 >mdiff.log
	@rem following test tests that we can use a directory as second argument
	-.\mdiff ../mdiff/tests/test1 ../mdiff/tests       >>mdiff.log
	-.\mdiff ../mdiff/tests/test1 ../mdiff/tests/test2 >>mdiff.log
	-.\mdiff ../mdiff/tests/test1 ../mdiff/tests/test3 >>mdiff.log
	-.\mdiff ../mdiff/tests/test1 ../mdiff/tests/test4 >>mdiff.log
	-.\mdiff ../mdiff/tests/test1 ../mdiff/tests/test5 >>mdiff.log
	-.\mdiff ../mdiff/tests/test6 ../mdiff/tests/test6extra >>mdiff.log
	-.\mdiff ../mdiff/tests/test7 ../mdiff/tests/test7extra >>mdiff.log
	@echo The following diff should give no output:
	diff mdiff.log ../mdiff/tests/mdiff.log
	..\tools\rm -f mdiff.log

test-thatch:
	@echo === test-thatch =================================================
	thatch ../data/test-image.jpg | tee thatch.log
	@echo The following mdiff and cmp should give no output:
	@echo TODO This sometimes fails (system dependent typeface in image?)
	..\tools\mdiff thatch.log ../tests
	cmp th-test-image.bmp ../tests/th-test-image.bmp
	rm -f thatch.log th-*.bmp

model-1.asm: tasm.exe ../data/tasm-68-1d.conf
	@echo === making model-1.asm ==========================================
	tasm -o model-1.asm ../data/tasm-68-1d.conf
	@echo The following mdiffs should give no output:
	..\tools\mdiff tasm.log ../tests/tasm-68-1d.log
	..\tools\mdiff model-1.asm ../data/model-1.asm
#       ..\tools\rm -f model-1.asm

model-2.asm: tasm.exe ../data/tasm-84-2d.conf
	@echo === making model-2.asm ==========================================
	tasm -o model-2.asm ../data/tasm-84-2d.conf
	@echo The following mdiffs should give no output:
	..\tools\mdiff tasm.log ../tests/tasm-84-2d.log
# TODO VjAv is slightly different in the DEBUG version
!IF "$(CFG)" == "Release"
	..\tools\mdiff model-2.asm ../data/model-2.asm
!ENDIF
#       ..\tools\rm -f model-2.asm

mu-68-1d.asm: tasm.exe ../data/tasm-mu-68-1d.conf
	@echo === making mu-68-1d.asm ==========================================
	tasm -o mu-68-1d.asm ../data/tasm-mu-68-1d.conf
	@echo The following mdiffs should give no output:
	..\tools\mdiff tasm.log ../tests/tasm-mu-68-1d.log
	..\tools\mdiff mu-68-1d.asm ../data/mu-68-1d.asm
#       ..\tools\rm -f mu-68-1d.asm

mu-76-2d.asm: tasm.exe ../data/tasm-mu-76-2d.conf
	@echo === making mu-76-2d.asm ==========================================
	tasm -o mu-76-2d.asm ../data/tasm-mu-76-2d.conf
	@echo The following mdiffs should give no output:
	..\tools\cp tasm.log ../tests/tasm-mu-76-2d.log
	..\tools\mdiff tasm.log ../tests/tasm-mu-76-2d.log
	..\tools\mdiff mu-76-2d.asm ../data/mu-76-2d.asm
#       ..\tools\rm -f mu-76-2d.asm

asm: model-1.asm model-2.asm mu-68-1d.asm mu-76-2d.asm

# TODO test-thatch removed from test because fails on some systems
test: test-stasm test-stasm-with-shapefile test-minimal test-dll \
  test-stasm-row test-tasm-example test-2d test-fdet 

# fulltest requires the XM2VTS and BioID images and includes the slow and esoteric tests
fulltest: test test-mdiff test-varied asm 

M2_MODELS=-c ../data/model-1.conf -c ../data/model-2.conf

# results for MUCT training set ("mutr" means "muct training set")

mutr-tabs:
	stasm -if -t tab/bioid-12-mutr.tab      ../data/bioid.shape  0              -p " B[^r]"
	stasm -if -t tab/bioid-12-mutr-easy.tab ../data/bioid.shape  0              -p " B[^r]" -P 102ff 0
	stasm -if -t tab/bioid-12-mutr-hard.tab ../data/bioid.shape  0              -p " B[^r]" -P 102ff ffffffff
	stasm -if -t tab/put-12-mutr.tab        ../data/put199.shape 0              -p " p[^r]"
 	stasm -if -t tab/mu-12-mutr.tab       ../data/muct68.shape 2000                       -p " i[^r]"
	stasm -if -t tab/ar-12-mutr.tab	      ../data/ar.shape     0                          -p " a[^r]"
	stasm -if -t tab/bioid-1-mutr-68.tab  ../data/bioid.shape  0 -c ../data/mu-68-1d.conf -p " B[^r]"
	stasm -if -t tab/bioid-2-mutr-76.tab  ../data/bioid.shape  0 -c ../data/mu-76-2d.conf -p " B[^r]"
	stasm -if -t tab/m2vts-12-muct.tab    ../data/xm68.shape   2000                       -p " m[^r]"

# results for XM2VTS training set

m2-tabs:
	stasm    -if -t tab/bioid-12.tab           ../data/bioid.shape 0     $(M2_MODELS)                  -p " B[^r]"
	stasm -r -if -t tab/bioid-row-12.tab       ../data/bioid.shape 0     $(M2_MODELS)                  -p " B[^r]"
	stasm    -if -t tab/ar-12.tab	           ../data/ar.shape 0        $(M2_MODELS)                  -p " a[^r]"
	stasm    -if -t tab/m2vts-12.tab           ../data/xm68.shape   2000 $(M2_MODELS)                  -p " m[^r]"
	stasm    -if -t tab/mu-12.tab              ../data/muct68.shape 2000 $(M2_MODELS)                  -p " i[^r]"
	stasm    -if -t tab/bioid-1.tab            ../data/bioid.shape 0 -c ../data/model-1.conf           -p " B[^r]"
	stasm    -if -t tab/bioid-1-vjmethod-1.tab ../data/bioid.shape 0 -c ../data/model-1-vjmethod1.conf -p " B[^r]"
	stasm -r -if -t tab/bioid-row-1.tab        ../data/bioid.shape 0 -c ../data/model-1.conf           -p " B[^r]"
	stasm    -if -t tab/ar-1.tab	           ../data/ar.shape    0 -c ../data/model-1.conf           -p " a[^r]"
	stasm -r -if -t tab/ar-row-1.tab	   ../data/ar.shape    0 -c ../data/model-1.conf           -p " a[^r]"
	stasm    -if -t tab/bioid-2.tab            ../data/bioid.shape 0 -c ../data/model-2.conf           -p " B[^r]"

tabs: mutr-tabs m2-tabs

# tar is for making local backups, currently hardwired for milbo's environment

tar:
	cd \a\stasm
	@rm -f stasm-temp.tar
	@rem exclude large files to keep archive size down
	C:\Rtools\bin\tar -cf stasm-temp.tar --exclude=*.lib --exclude=*.dll --exclude=*.xml --exclude=*.tag --exclude=x* stasm
	@echo contents of archive, sorted on size:
	C:\Rtools\bin\tar -tvf stasm-temp.tar | C:\Rtools\bin\sort.exe -g -k 4
	@rm -f stasm-temp.tar.gz
	gzip stasm-temp.tar
	ls -l stasm-temp.tar.gz
	mv stasm-temp.tar.gz /a/homepage/milbo.org/tar
	cd \a\stasm\stasm

OBJ=\
	$(OUTDIR)/atface.obj\
	$(OUTDIR)/ezfont.obj\
	$(OUTDIR)/find.obj\
	$(OUTDIR)/follow.obj\
	$(OUTDIR)/forward.obj\
	$(OUTDIR)/hypot.obj\
	$(OUTDIR)/imfile.obj\
	$(OUTDIR)/imgiven.obj\
	$(OUTDIR)/imshape.obj\
	$(OUTDIR)/imutil.obj\
	$(OUTDIR)/imwrite.obj\
	$(OUTDIR)/initnet.obj\
	$(OUTDIR)/jpegutil.obj\
	$(OUTDIR)/landmarks.obj\
	$(OUTDIR)/mat.obj\
	$(OUTDIR)/matvec.obj\
	$(OUTDIR)/mchol.obj\
	$(OUTDIR)/mrand.obj\
	$(OUTDIR)/pngutil.obj\
	$(OUTDIR)/prof.obj\
	$(OUTDIR)/readconf.obj\
	$(OUTDIR)/readpng.obj\
	$(OUTDIR)/regex.obj\
	$(OUTDIR)/rgbimutil.obj\
	$(OUTDIR)/rowley.obj\
	$(OUTDIR)/rowleyhand.obj\
	$(OUTDIR)/safe_alloc.obj\
	$(OUTDIR)/search.obj\
	$(OUTDIR)/shapefile.obj\
	$(OUTDIR)/shapemodel.obj\
	$(OUTDIR)/sparsemat.obj\
	$(OUTDIR)/startshape.obj\
	$(OUTDIR)/stasm.obj\
	$(OUTDIR)/tclHash.obj\
	$(OUTDIR)/util.obj\
	$(OUTDIR)/violajones.obj\
	$(OUTDIR)/vjhand.obj\
	$(OUTDIR)/wrbmp.obj

STASM_OBJ=\
	$(OBJ)\
	$(OUTDIR)/asmsearch.obj\
	$(OUTDIR)/initasm.obj\
	$(OUTDIR)/readasm.obj\
	$(OUTDIR)/tab.obj

STASM_MAIN_OBJ=\
	$(OUTDIR)/err.obj\
	$(OUTDIR)/main.obj\
	$(OUTDIR)/release.obj\
	$(STASM_OBJ)

MINIMAL_OBJ=\
	$(OUTDIR)/err.obj\
	$(OUTDIR)/minimal.obj\
	$(OUTDIR)/release.obj\
	$(STASM_OBJ)

STASM_DLL_OBJ=\
	$(OUTDIR)/stasm_dll.obj\
	$(OUTDIR)/err.obj\
	$(OUTDIR)/release.obj\
	$(STASM_OBJ)

TASM_OBJ=\
	$(OUTDIR)/detav.obj\
	$(OUTDIR)/err.obj\
	$(OUTDIR)/eyesynth.obj\
	$(OUTDIR)/proftrain.obj\
	$(OUTDIR)/release.obj\
	$(OUTDIR)/tasm.obj\
	$(OUTDIR)/tasmshapes.obj\
	$(OUTDIR)/tcovar.obj\
	$(OBJ)

WASM_OBJ=\
	$(OUTDIR)/wasm.obj\
	$(OUTDIR)/wasm.res\
	$(OUTDIR)/werr.obj\
	$(OUTDIR)/winfile.obj\
	$(OUTDIR)/wrelease.obj\
	$(STASM_OBJ)

CASM_OBJ=\
	$(OUTDIR)/camera.obj\
	$(OUTDIR)/casm.obj\
	$(OUTDIR)/casm.res\
	$(OUTDIR)/werr.obj\
	$(OUTDIR)/winfile.obj\
	$(OUTDIR)/wrelease.obj\
	$(STASM_OBJ)

WINTHATCH_OBJ=\
	$(OUTDIR)/camera.obj\
	$(OUTDIR)/libthatch.obj\
	$(OUTDIR)/werr.obj\
	$(OUTDIR)/winfile.obj\
	$(OUTDIR)/winthatch.obj\
	$(OUTDIR)/winthatch.res\
	$(OUTDIR)/wrelease.obj\
	$(STASM_OBJ)

THATCH_OBJ=\
	$(OUTDIR)/err.obj\
	$(OUTDIR)/libthatch.obj\
	$(OUTDIR)/release.obj\
	$(OUTDIR)/thatch.obj\
	$(STASM_OBJ)

FDET_OBJ=\
	$(OUTDIR)/err.obj\
	$(OUTDIR)/fdet.obj\
	$(OUTDIR)/release.obj\
	$(STASM_OBJ)

SHAPE2R_OBJ=\
	$(OUTDIR)/err.obj\
	$(OUTDIR)/release.obj\
	$(OUTDIR)/shape2r.obj\
	$(STASM_OBJ)

MARKI_OBJ=\
	$(OUTDIR)/imequalize.obj\
	$(OUTDIR)/marki.obj\
	$(OUTDIR)/marki.res\
	$(OUTDIR)/werr.obj\
	$(OUTDIR)/winfile.obj\
	$(OUTDIR)/wrelease.obj\
	$(STASM_OBJ)

ILAND_OBJ=\
	$(OUTDIR)/asmsearch1.obj\
	$(OUTDIR)/iland.obj\
	$(OUTDIR)/iland.res\
	$(OUTDIR)/imequalize.obj\
	$(OUTDIR)/werr.obj\
	$(OUTDIR)/winfile.obj\
	$(OUTDIR)/wrelease.obj\
	$(STASM_OBJ)

# using local copies of DLLs is a brute force way of avoiding DLL path problems

cv210.dll:
	..\tools\cp $(OPENCV_HOME)/bin/cv210.dll .

cxcore210.dll:
	..\tools\cp $(OPENCV_HOME)/bin/cxcore210.dll .

highgui210.dll:
	..\tools\cp $(OPENCV_HOME)/bin/highgui210.dll .

stasm.exe: $(STASM_MAIN_OBJ) $(LIBS) $(OPENCV_DLLS)
	link -out:stasm.exe $(LFLAGS) $(STASM_MAIN_OBJ) $(SETARGV) $(BINMODE) $(LIBS) $(WINLIBS)

minimal.exe: $(MINIMAL_OBJ) $(LIBS) $(OPENCV_DLLS)
	link -out:minimal.exe $(LFLAGS) $(MINIMAL_OBJ) $(BINMODE) $(LIBS) $(WINLIBS)

stasm_dll.dll: $(STASM_DLL_OBJ) $(LIBS) $(OPENCV_DLLS)
	link /DLL $(LFLAGS) $(STASM_DLL_OBJ) $(BINMODE) $(LIBS) $(WINLIBS)

../data/stasm_dll.dll: stasm_dll.dll
	..\tools\cp stasm_dll.dll ../data

../data/stasm_dll.lib: stasm_dll.dll
	..\tools\cp stasm_dll.lib ../data

stasm_opencv_example.exe: $(OUTDIR)/stasm_opencv_example.obj ../data/stasm_dll.dll ../data/stasm_dll.lib
	link -out:stasm_opencv_example.exe $(LFLAGS) $(OUTDIR)/stasm_opencv_example.obj $(SETARGV) $(BINMODE) $(LIBS) ..\data\stasm_dll.lib

testdll.exe: $(OUTDIR)/testdll.obj ../data/stasm_dll.dll ../data/stasm_dll.lib
	link -out:testdll.exe $(LFLAGS) $(OUTDIR)/testdll.obj $(SETARGV) $(BINMODE) $(LIBS) ..\data\stasm_dll.lib

tasm.exe: $(TASM_OBJ) $(LIBS) $(OPENCV_DLLS)
	link -out:tasm.exe $(LFLAGS) $(TASM_OBJ) $(SETARGV) $(BINMODE) $(LIBS) $(WINLIBS)

marki.exe: $(MARKI_OBJ) $(LIBS)
	link -out:marki.exe -subsystem:windows $(LFLAGS) $(MARKI_OBJ) $(SETARGV) $(BINMODE) $(LIBS) $(WINLIBS)

iland.exe: $(ILAND_OBJ) $(LIBS) $(OPENCV_DLLS)
	link -out:iland.exe -subsystem:windows $(LFLAGS) $(ILAND_OBJ) $(SETARGV) $(BINMODE) $(LIBS) $(WINLIBS)

fdet.exe: $(FDET_OBJ) $(LIBS) $(OPENCV_DLLS)
	link -out:fdet.exe $(LFLAGS) $(FDET_OBJ) $(SETARGV) $(BINMODE) $(LIBS) $(WINLIBS)

shape2r.exe: $(SHAPE2R_OBJ) $(LIBS) $(OPENCV_DLLS)
	link -out:shape2r.exe $(LFLAGS) $(SHAPE2R_OBJ) $(SETARGV) $(BINMODE) $(LIBS) $(WINLIBS)

wasm.exe: $(WASM_OBJ) $(LIBS) $(OPENCV_DLLS)
	link -out:wasm.exe  -subsystem:windows $(LFLAGS) $(WASM_OBJ) $(SETARGV) $(BINMODE) $(LIBS) $(WINLIBS)

casm.exe: $(CASM_OBJ) $(LIBS) $(OPENCV_DLLS) ../winapps/casm/casm.hpp ../winapps/casm/camera.hpp ../stasm/winfile.hpp
	link -out:casm.exe  -subsystem:windows $(LFLAGS) $(CASM_OBJ) $(SETARGV) $(BINMODE) $(LIBS) $(WINLIBS)

thatch.exe: $(THATCH_OBJ) $(LIBS) $(OPENCV_DLLS)
	link -out:thatch.exe $(LFLAGS) $(THATCH_OBJ) $(SETARGV) $(BINMODE) $(LIBS) $(WINLIBS)

winthatch.exe: $(WINTHATCH_OBJ) $(LIBS) $(OPENCV_DLLS)
	link -out:winthatch.exe  -subsystem:windows $(LFLAGS) $(WINTHATCH_OBJ) $(SETARGV) $(BINMODE) $(LIBS) $(WINLIBS)

mdiff.exe: ../mdiff/mdiff.cpp
	$(CC) -nologo -O2 -W3 ..\mdiff\mdiff.cpp $(SETARGV) $(BINMODE) 
	..\tools\rm -f mdiff.obj

../stasm/stasm.hpp:\
	../image/image.hpp\
	../image/imfile.hpp\
	../image/imwrite.hpp\
	../image/imgiven.hpp\
	../image/imutil.hpp\
	../image/jpegutil.hpp\
	../image/pngutil.hpp\
	../image/rgbimutil.hpp\
	../mat/mat.hpp\
	../mat/matvec.hpp\
	../mat/matview.hpp\
	../mat/mchol.hpp\
	../rowley/find.hpp\
	../rowley/follow.hpp\
	../rowley/forward.hpp\
	../rowley/initnet.hpp\
	../rowley/list.hpp\
	../rowley/search.hpp\
	../stasm/asmsearch.hpp\
	../stasm/atface.hpp\
	../stasm/colors.hpp\
	../stasm/err.hpp\
	../stasm/imshape.hpp\
	../stasm/initasm.hpp\
	../stasm/landmarks.hpp\
	../stasm/mouth.hpp\
	../stasm/safe_alloc.hpp\
	../stasm/me17s.hpp\
	../stasm/prof.hpp\
	../stasm/readasm.hpp\
	../stasm/readconf.hpp\
	../stasm/rowley.hpp\
	../stasm/rowleyhand.hpp\
	../stasm/shapefile.hpp\
	../stasm/shapemodel.hpp\
	../stasm/sparsemat.hpp\
	../stasm/startshape.hpp\
	../stasm/tab.hpp\
	../stasm/util.hpp\
	../stasm/violajones.hpp\
	../stasm/vjhand.hpp\
	../tasm/detav.hpp\
	../tasm/eyesynth.hpp\
	../tasm/mrand.hpp\
	../tasm/mrand.hpp\
	../tasm/proftrain.hpp\
	../tasm/tasm.hpp\
	../tasm/tasmshapes.hpp\
	../tasm/tcovar.hpp

$(OUTDIR)/marki.res: ../winapps/marki/marki.rc ../winapps/marki/marki.hpp ../winapps/marki/marki.bmp ../winapps/marki/marki.ico
	rc -r -DWIN32 ../winapps/marki/marki.rc
	..\tools\mv ../winapps/marki/marki.res $(OUTDIR)/marki.res

$(OUTDIR)/iland.res: ../winapps/iland/iland.rc ../winapps/iland/iland.hpp ../winapps/iland/iland.bmp ../winapps/iland/iland.ico
	rc -r -DWIN32 ../winapps/iland/iland.rc
	..\tools\mv ../winapps/iland/iland.res $(OUTDIR)/iland.res

$(OUTDIR)/wasm.res: ../winapps/wasm/wasm.rc ../winapps/wasm/wasm.hpp ../winapps/wasm/wasm.bmp ../winapps/wasm/wasm.ico
	rc -r -DWIN32 ../winapps/wasm/wasm.rc
	..\tools\mv ../winapps/wasm/wasm.res $(OUTDIR)/wasm.res

$(OUTDIR)/casm.res: ../winapps/casm/casm.rc ../winapps/casm/casm.hpp ../winapps/wasm/wasm.bmp ../winapps/casm/casm.ico
	rc -r -DWIN32 ../winapps/casm/casm.rc
	..\tools\mv ../winapps/casm/casm.res $(OUTDIR)/casm.res

$(OUTDIR)/winthatch.res: ../winapps/thatch/winthatch.rc ../winapps/thatch/winthatch.hpp ../winapps/thatch/libthatch.hpp ../winapps/thatch/winthatch.bmp ../winapps/thatch/winthatch.ico
	rc -r -DWIN32 ../winapps/thatch/winthatch.rc
	..\tools\mv ../winapps/thatch/winthatch.res $(OUTDIR)/winthatch.res

$(OUTDIR)/asmsearch.obj: ../stasm/asmsearch.cpp ../stasm/stasm.hpp
	cl -c ../stasm/asmsearch.cpp $(CFLAGS)

$(OUTDIR)/asmsearch1.obj: ../winapps/iland/asmsearch1.cpp ../stasm/stasm.hpp ../winapps/iland/asmsearch1.hpp
	cl -c ../winapps/iland/asmsearch1.cpp $(CFLAGS)

$(OUTDIR)/atface.obj: ../stasm/atface.cpp ../stasm/stasm.hpp
	cl -c ../stasm/atface.cpp $(CFLAGS)

$(OUTDIR)/err.obj: ../stasm/err.cpp ../stasm/stasm.hpp
	cl -c ../stasm/err.cpp $(CFLAGS)

$(OUTDIR)/imshape.obj: ../stasm/imshape.cpp ../stasm/stasm.hpp
	cl -c ../stasm/imshape.cpp $(CFLAGS)

$(OUTDIR)/initasm.obj: ../stasm/initasm.cpp ../stasm/stasm.hpp
	cl -c ../stasm/initasm.cpp $(CFLAGS)

$(OUTDIR)/landmarks.obj: ../stasm/landmarks.cpp ../stasm/stasm.hpp
	cl -c ../stasm/landmarks.cpp $(CFLAGS)

$(OUTDIR)/main.obj: ../stasm/main.cpp ../stasm/stasm.hpp
	cl -c ../stasm/main.cpp $(CFLAGS)

$(OUTDIR)/prof.obj: ../stasm/prof.cpp ../stasm/stasm.hpp
	cl -c ../stasm/prof.cpp $(CFLAGS)

$(OUTDIR)/readasm.obj: ../stasm/readasm.cpp ../stasm/stasm.hpp
	cl -c ../stasm/readasm.cpp $(CFLAGS)

$(OUTDIR)/readconf.obj: ../stasm/readconf.cpp ../stasm/stasm.hpp
	cl -c ../stasm/readconf.cpp $(CFLAGS)

$(OUTDIR)/release.obj: ../stasm/release.cpp ../stasm/stasm.hpp
	cl -c ../stasm/release.cpp $(CFLAGS)

$(OUTDIR)/rowley.obj: ../stasm/rowley.cpp ../stasm/stasm.hpp
	cl -c ../stasm/rowley.cpp $(CFLAGS)

$(OUTDIR)/rowleyhand.obj: ../stasm/rowleyhand.cpp ../stasm/stasm.hpp
	cl -c ../stasm/rowleyhand.cpp $(CFLAGS)

$(OUTDIR)/safe_alloc.obj: ../stasm/safe_alloc.cpp ../stasm/stasm.hpp
	cl -c ../stasm/safe_alloc.cpp $(CFLAGS)

$(OUTDIR)/shapefile.obj: ../stasm/shapefile.cpp ../stasm/stasm.hpp
	cl -c ../stasm/shapefile.cpp $(CFLAGS)

$(OUTDIR)/shapemodel.obj: ../stasm/shapemodel.cpp ../stasm/stasm.hpp
	cl -c ../stasm/shapemodel.cpp $(CFLAGS)

$(OUTDIR)/sparsemat.obj: ../stasm/sparsemat.cpp ../stasm/stasm.hpp
	cl -c ../stasm/sparsemat.cpp $(CFLAGS)

$(OUTDIR)/startshape.obj: ../stasm/startshape.cpp ../stasm/stasm.hpp
	cl -c ../stasm/startshape.cpp $(CFLAGS)

$(OUTDIR)/stasm.obj: ../stasm/stasm.cpp ../stasm/stasm.hpp
	cl -c ../stasm/stasm.cpp $(CFLAGS)

$(OUTDIR)/tab.obj: ../stasm/tab.cpp ../stasm/stasm.hpp
	cl -c ../stasm/tab.cpp $(CFLAGS)

$(OUTDIR)/util.obj: ../stasm/util.cpp ../stasm/stasm.hpp
	cl -c ../stasm/util.cpp $(CFLAGS)

$(OUTDIR)/violajones.obj: ../stasm/violajones.cpp ../stasm/stasm.hpp ../stasm/stasm_cv.h
	cl -c ../stasm/violajones.cpp $(CFLAGS)

$(OUTDIR)/vjhand.obj: ../stasm/vjhand.cpp ../stasm/stasm.hpp
	cl -c ../stasm/vjhand.cpp $(CFLAGS)

$(OUTDIR)/werr.obj: ../stasm/werr.cpp ../stasm/stasm.hpp
	cl -c ../stasm/werr.cpp $(CFLAGS)

$(OUTDIR)/winfile.obj: ../stasm/winfile.cpp ../stasm/stasm.hpp ../stasm/winfile.hpp
	cl -c ../stasm/winfile.cpp $(CFLAGS)

$(OUTDIR)/wrelease.obj: ../stasm/wrelease.cpp ../stasm/stasm.hpp
	cl -c ../stasm/wrelease.cpp $(CFLAGS)

$(OUTDIR)/fdet.obj: ../tools/fdet.cpp ../stasm/stasm.hpp
	cl -c ../tools/fdet.cpp $(CFLAGS)

$(OUTDIR)/shape2r.obj: ../tools/shape2r.cpp ../stasm/stasm.hpp
	cl -c ../tools/shape2r.cpp $(CFLAGS)

$(OUTDIR)/wasm.obj: ../winapps/wasm/wasm.cpp ../stasm/stasm.hpp ../winapps/wasm/wasm.hpp ../stasm/winfile.hpp
	cl -c ../winapps/wasm/wasm.cpp $(CFLAGS)

$(OUTDIR)/casm.obj: ../winapps/casm/casm.cpp ../stasm/stasm.hpp ../winapps/casm/casm.hpp ../winapps/casm/camera.hpp ../stasm/winfile.hpp
	cl -c ../winapps/casm/casm.cpp $(CFLAGS)

$(OUTDIR)/thatch.obj: ../winapps/thatch/thatch.cpp ../stasm/stasm.hpp ../winapps/casm/camera.hpp ../winapps/thatch/libthatch.hpp
	cl -c ../winapps/thatch/thatch.cpp $(CFLAGS)

$(OUTDIR)/winthatch.obj: ../winapps/thatch/winthatch.cpp ../stasm/stasm.hpp ../winapps/thatch/winthatch.hpp ../winapps/casm/camera.hpp ../winapps/thatch/libthatch.hpp ../stasm/winfile.hpp
	cl -c ../winapps/thatch/winthatch.cpp $(CFLAGS)

$(OUTDIR)/libthatch.obj: ../winapps/thatch/libthatch.cpp ../stasm/stasm.hpp ../winapps/thatch/libthatch.hpp ../winapps/casm/camera.hpp ../winapps/thatch/libthatch.hpp
	cl -c ../winapps/thatch/libthatch.cpp $(CFLAGS)

$(OUTDIR)/camera.obj: ../winapps/casm/camera.cpp ../stasm/stasm.hpp ../winapps/casm/casm.hpp ../winapps/casm/camera.hpp
	cl -c ../winapps/casm/camera.cpp $(CFLAGS)

$(OUTDIR)/marki.obj: ../winapps/marki/marki.cpp ../stasm/stasm.hpp ../winapps/marki/marki.hpp ../image/imequalize.hpp ../stasm/winfile.hpp
	cl -c ../winapps/marki/marki.cpp $(CFLAGS)

$(OUTDIR)/iland.obj: ../winapps/iland/iland.cpp ../stasm/stasm.hpp ../winapps/iland/iland.hpp ../winapps/iland/asmsearch1.hpp ../stasm/winfile.hpp
	cl -c ../winapps/iland/iland.cpp $(CFLAGS)

$(OUTDIR)/minimal.obj: ../minimal/minimal.cpp ../stasm/stasm.hpp
	cl -c ../minimal/minimal.cpp $(CFLAGS)

$(OUTDIR)/eyesynth.obj: ../tasm/eyesynth.cpp ../stasm/stasm.hpp ../tasm/tasm.hpp
	cl -c ../tasm/eyesynth.cpp $(CFLAGS)

$(OUTDIR)/mat.obj: ../mat/mat.cpp ../stasm/stasm.hpp
	cl -c ../mat/mat.cpp $(CFLAGS)

$(OUTDIR)/mchol.obj: ../mat/mchol.cpp ../stasm/stasm.hpp
	cl -c ../mat/mchol.cpp $(CFLAGS)

$(OUTDIR)/matview.obj: ../mat/matview.cpp ../stasm/stasm.hpp
	cl -c ../mat/matview.cpp $(CFLAGS)

$(OUTDIR)/matvec.obj: ../mat/matvec.cpp ../stasm/stasm.hpp
	cl -c ../mat/matvec.cpp $(CFLAGS)

$(OUTDIR)/imequalize.obj: ../image/imequalize.cpp ../stasm/stasm.hpp
	cl -c ../image/imequalize.cpp $(CFLAGS)

$(OUTDIR)/imfile.obj: ../image/imfile.cpp ../stasm/stasm.hpp
	cl -c ../image/imfile.cpp $(CFLAGS)

$(OUTDIR)/imwrite.obj: ../image/imwrite.cpp ../stasm/stasm.hpp
	cl -c ../image/imwrite.cpp $(CFLAGS)

$(OUTDIR)/imgiven.obj: ../image/imgiven.cpp ../stasm/stasm.hpp
	cl -c ../image/imgiven.cpp $(CFLAGS)

$(OUTDIR)/imutil.obj: ../image/imutil.cpp ../stasm/stasm.hpp
	cl -c ../image/imutil.cpp $(CFLAGS)

$(OUTDIR)/rgbimutil.obj: ../image/rgbimutil.cpp ../stasm/stasm.hpp
	cl -c ../image/rgbimutil.cpp $(CFLAGS)

$(OUTDIR)/ezfont.obj: ../image/ezfont.c ../stasm/stasm.hpp
	cl -c ../image/ezfont.c $(CFLAGS)

$(OUTDIR)/jpegutil.obj: ../image/jpegutil.cpp ../stasm/stasm.hpp
	cl -c ../image/jpegutil.cpp $(CFLAGS)

$(OUTDIR)/forward.obj: ../rowley/forward.cpp ../stasm/stasm.hpp
	cl -c ../rowley/forward.cpp $(CFLAGS)

$(OUTDIR)/follow.obj: ../rowley/follow.cpp ../stasm/stasm.hpp
	cl -c ../rowley/follow.cpp $(CFLAGS)

$(OUTDIR)/imu.obj: ../rowley/imu.cpp ../stasm/stasm.hpp
	cl -c ../rowley/imu.cpp $(CFLAGS)

$(OUTDIR)/find.obj: ../rowley/find.cpp ../stasm/stasm.hpp
	cl -c ../rowley/find.cpp $(CFLAGS)

$(OUTDIR)/hypot.obj: ../stasm/hypot.c
	cl -c ../stasm/hypot.c $(CFLAGS)

$(OUTDIR)/initnet.obj: ../rowley/initnet.cpp ../stasm/stasm.hpp
	cl -c ../rowley/initnet.cpp $(CFLAGS)

$(OUTDIR)/search.obj: ../rowley/search.cpp ../stasm/stasm.hpp ../rowley/list.hpp
	cl -c ../rowley/search.cpp $(CFLAGS)

$(OUTDIR)/wrbmp.obj: ../jpeg/wrbmp.c ../stasm/stasm.hpp
	cl -c ../jpeg/wrbmp.c $(CFLAGS)

$(OUTDIR)/regex.obj: ../regex/regex.c ../regex/regex.h ../stasm/stasm.hpp
	cl -c ../regex/regex.c $(REGEX_DEFS) $(CFLAGS)

$(OUTDIR)/pngutil.obj: ../image/pngutil.cpp ../stasm/stasm.hpp
	cl -c ../image/pngutil.cpp $(CFLAGS)

$(OUTDIR)/readpng.obj: ../png/readpng.c ../png/readpng.h ../stasm/stasm.hpp
	cl -c ../png/readpng.c $(CFLAGS)

$(OUTDIR)/proftrain.obj: ../tasm/proftrain.cpp ../stasm/stasm.hpp ../tasm/tasm.hpp
	cl -c ../tasm/proftrain.cpp $(CFLAGS)

$(OUTDIR)/tasm.obj: ../tasm/tasm.cpp ../stasm/stasm.hpp ../tasm/tasm.hpp ../tasm/tasmshapes.hpp ../tasm/detav.hpp
	cl -c ../tasm/tasm.cpp $(CFLAGS)

$(OUTDIR)/detav.obj: ../tasm/detav.cpp ../stasm/stasm.hpp ../tasm/detav.hpp
	cl -c ../tasm/detav.cpp $(CFLAGS)

$(OUTDIR)/tasmshapes.obj: ../tasm/tasmshapes.cpp ../stasm/stasm.hpp ../tasm/tasm.hpp
	cl -c ../tasm/tasmshapes.cpp $(CFLAGS)

$(OUTDIR)/tcovar.obj: ../tasm/tcovar.cpp ../stasm/stasm.hpp
	cl -c ../tasm/tcovar.cpp $(CFLAGS)

$(OUTDIR)/list.obj: ../rowley/list.cpp ../rowley/list.hpp
	cl -c ../rowley/list.cpp $(CFLAGS)

$(OUTDIR)/tclHash.obj: ../rowley/tclHash.c ../rowley/tclHash.h
	cl -c ../rowley/tclHash.c $(CFLAGS)

$(OUTDIR)/mrand.obj: ../tasm/mrand.cpp ../tasm/mrand.hpp
	cl -c ../tasm/mrand.cpp $(CFLAGS)

$(OUTDIR)/stasm_dll.obj: ../stasm/stasm_dll.cpp ../stasm/stasm_dll.hpp
	cl -c ../stasm/stasm_dll.cpp $(CFLAGS)

$(OUTDIR)/stasm_opencv_example.obj: ../opencv_example/stasm_opencv_example.cpp
	cl -c ../opencv_example/stasm_opencv_example.cpp $(CFLAGS)

$(OUTDIR)/testdll.obj: ../stasm/testdll.cpp
	cl -c ../stasm/testdll.cpp $(CFLAGS)
