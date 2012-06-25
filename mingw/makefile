# $stasm/mingw/makefile.mingw 3.0 milbo$ mingw GNU gcc makefile for stasm
#
# To build: make -f makefile
#
# TODO dependencies aren't correct -- why do we need touch for stasm.hpp?
#
# milbo Forden Aug 08

all: mdiff.exe stasm.exe tasm.exe minimal.exe fdet.exe thatch.exe

# modify the following for your environment

LIBS=\
    /Rtools/MinGW/lib/gcc/mingw32/4.2.1-sjlj/libstdc++.a\
    cv210.dll\
    cxcore210.dll\
    highgui210.dll\
    libgsl.dll\
    libgslcblas.dll\
    jpeg62.dll\

BIOID_IMAGES= /a1/faces/bioid/lpgm/B*.pgm
AR_IMAGES=    /a1/faces/ar/pgm/a*.pgm
XM2VTS_IMAGES=/a1/faces/m2/pgm/m*.pgm
# use a subset of 1200 XM2VTS images for testing, takes too long to test all
XM2VTS_TEST_IMAGES=/a1/faces/m2/pgm/m??1*.pgm /a1/faces/m2/pgm/m??3*.pgm \
	      /a1/faces/m2/pgm/m??5*.pgm /a1/faces/m2/pgm/m??7*.pgm \
	      /a1/faces/m2/pgm/m??9*.pgm

OPENCV_HOME=C:/OpenCV2.1

OPENCV_DLLS=cv210.dll cxcore210.dll highgui210.dll

ALL_IMAGES=$(BIOID_IMAGES) $(XM2VTS_IMAGES) $(AR_IMAGES) /a1/faces/mirror/pgm/*.pgm

REGEX_DEFS=-DHAVE_CONFIG_H -DSTDC_HEADERS -DREGEX_MALLOC -c

INCL=\
    -I$(OPENCV_HOME)/include/opencv\
    -I../regex\
    -I../gsl\
    -I../gsl/gsl\
    -I../image\
    -I../jpeg\
    -I../mat\
    -I../rowley\
    -I../stasm\
    -I../tasm

# -Wno-long-long disables the "long long warnings" for the OpenCV headers
CFLAGS=-O3 -Wall -pedantic -Wno-long-long -DWIN32 $(INCL)
LFLAGS=$(CFLAGS)
CC=gcc
LINK=g++

clean:
	rm -f *.dll *.tab *.exe *._xe 
	rm -f *.o *.log *.map *.opt *.pdb *.ilk *.ps *.pbi *.pbo *.pbt *.pch

veryclean: clean
	rm -f *.asm *.bmp *.log *.conf *.tab *.profileResults
	rm -f Wasm-log.txt out/fdet-*.bmp search-results.bmp
	rm -f ../*/default.tag *.tar *.gz _*.shape *.dll

# generate face detector parameters for manual insertion into .shape file

vj-det-params:
	@echo === vj-det-params ===============================================
	./fdet $(ALL_IMAGES)

rowley-det-params:
	@echo === rowley-det-params ===========================================
	./fdet -r $(ALL_IMAGES)

# If any test below fails the make will abort with an error message

test-stasm: # stasm test with default config files and three images
	@echo === test-stasm ==================================================
	@echo === expect a warning for no-face.jpg
	./stasm -iS ../tests/B1138_20.pgm ../tests/no-face.jpg ../data/test-image.jpg
	@echo The following mdiffs and cmp should give no output:
	../tools/mdiff stasm.log ../tests/test-stasm.log
	cmp 1-B1138_20.bmp ../tests/1-B1138_20.bmp
	cmp s-B1138_20.bmp ../tests/s-B1138_20.bmp
	rm -f stasm.log *-B1138_20.bmp s-no-face-noface.bmp 1-test-image.bmp s-test-image.bmp

test-stasm-with-shapefile: # stasm test with reference shapefile
	@echo === test-stasm-with-shapefile ===================================
	./stasm -if -t test.tab ../data/bioid.shape 0 -iS -p "B1138"
	@echo The following mdiff should give no output:
	../tools/mdiff stasm.log ../tests/stasm-test-with-shapefile.log
	../tools/mdiff test.tab ../tests/stasm-test-with-shapefile.tab
	rm -f stasm.log test.tab *-B1138_20.bmp 

test-minimal:
	@echo === test-minimal ================================================
	./minimal
	@echo The following cmp should give no output:
	cmp search-results.bmp ../tests/minimal-search-results.bmp
	rm -f search-results.bmp

test-stasm-row: # stasm rowley test with default config files
	@echo === test-stasm-row ==============================================
	./stasm -r -iS ../data/test-image.jpg
	@echo The following mdiff and cmps should give no output:
	../tools/mdiff stasm.log ../tests/mingw-test-row-stasm.log
	cmp 1-row-test-image.bmp ../tests/1-row-test-image.bmp
	cmp s-row-test-image.bmp ../tests/s-row-test-image.bmp
	rm -f stasm.log *-test-image.bmp

# The following is an example of how to use tasm on non XM2VTS shapes
# It is also a good test of the tasm and stasm software

test-tasm-example:
	@echo === test-tasm-example ===========================================
	./tasm -o example.asm ../tasm-example/tasm-example.conf
	@echo The following mdiffs should give no output:
	../tools/mdiff tasm.log ../tasm-example/tests/tasm-example.log
	../tools/mdiff example.asm ../tasm-example/tests/mingw-example.asm
	@echo === stasm test with above example asm file =====================
	./stasm -iS -c ../tasm-example/example.conf ../data/test-image.jpg
	@echo The following mdiff and cmps should give no output:
	../tools/mdiff stasm.log ../tasm-example/tests/mingw-vj-stasm.log
	cmp 1-test-image.bmp ../tasm-example/tests/1-test-image.bmp
	cmp s-test-image.bmp ../tasm-example/tests/mingw-s-test-image.bmp
	rm -f tasm.log stasm.log example.asm *-test-image.bmp

test-fdet:
	@echo === test-fdet ===================================================
	./fdet ../data/test-image.jpg
	@echo The following mdiffs and cmp should give no output:
	../tools/mdiff fdet.log ../tests/test-fdet.log
	rm -f fdet.log

test-mdiff:
	@echo === test-mdiff ==================================================
	# the - prefix means ignore correct "error" returns for non matches
	../tools/mdiff ../mdiff/tests/test1 ../mdiff/tests/test1 >mdiff.log
	# following test tests that we can use a directory as second argument
	-../tools/mdiff ../mdiff/tests/test1 ../mdiff/tests       >>mdiff.log
	-../tools/mdiff ../mdiff/tests/test1 ../mdiff/tests/test2 >>mdiff.log
	-../tools/mdiff ../mdiff/tests/test1 ../mdiff/tests/test3 >>mdiff.log
	-../tools/mdiff ../mdiff/tests/test1 ../mdiff/tests/test4 >>mdiff.log
	-../tools/mdiff ../mdiff/tests/test1 ../mdiff/tests/test5 >>mdiff.log
	-../tools/mdiff ../mdiff/tests/test6 ../mdiff/tests/test6extra >>mdiff.log
	-../tools/mdiff ../mdiff/tests/test7 ../mdiff/tests/test7extra >>mdiff.log
	@echo The following diff should give no output:
	diff mdiff.log ../mdiff/tests/mdiff.log
	../tools/rm -f mdiff.log

test-thatch:
	@echo === test-thatch =================================================
	./thatch ../data/test-image.jpg | tee thatch.log
	@echo The following mdiff and cmp should give no output:
	../tools/mdiff thatch.log ../tests
	cmp th-test-image.bmp ../tests/mingw-th-test-image.bmp
	rm -f thatch.log th-*.bmp

test: test-stasm test-stasm-with-shapefile test-minimal test-stasm-row test-tasm-example test-fdet test-mdiff test-thatch

model-1.asm: tasm.exe ../data/tasm-68-1d.conf
	@echo === making model-1.asm ==========================================
	./tasm -o model-1.asm ../data/tasm-68-1d.conf

model-2.asm: tasm.exe ../data/tasm-84-2d.conf
	@echo === making model-2.asm ==========================================
	./tasm -o model-2.asm ../data/tasm-84-2d.conf

mu-68-1d.asm: tasm.exe ../data/tasm-mu-68-1d.conf
	@echo === making mu-68-1d.asm ==========================================
	./tasm -o mu-68-1d.asm ../data/tasm-mu-68-1d.conf

mu-76-2d.asm: tasm.exe ../data/tasm-mu-76-2d.conf
	@echo === making mu-76-2d.asm ==========================================
	./tasm -o mu-76-2d.asm ../data/tasm-mu-76-2d.conf

OBJ=\
	stasm.o\
	atface.o\
	ezfont.o\
	find.o\
	follow.o\
	forward.o\
	imfile.o\
	imwrite.o\
	imgiven.o\
	imshape.o\
	imutil.o\
	initnet.o\
	jpegutil.o\
	landmarks.o\
	mat.o\
	matvec.o\
	mchol.o\
	mrand.o\
	prof.o\
	readconf.o\
	regex.o\
	rgbimutil.o\
	rowley.o\
	rowleyhand.o\
	search.o\
	shapefile.o\
	shapemodel.o\
	sparsemat.o\
	startshape.o\
	safe_alloc.o\
	tclHash.o\
	util.o\
	violajones.o\
	vjhand.o\
	wrbmp.o

STASM_OBJ=\
	$(OBJ)\
	asmsearch.o\
	initasm.o\
	readasm.o\
	tab.o

STASM_MAIN_OBJ=\
	main.o\
	err.o\
	release.o\
	$(STASM_OBJ)

MINIMAL_OBJ=\
	minimal.o\
	err.o\
	release.o\
	$(STASM_OBJ)

TASM_OBJ=\
	tasm.o\
	detav.o\
	eyesynth.o\
	proftrain.o\
	tasmshapes.o\
	tcovar.o\
	release.o\
	err.o\
	$(OBJ)

WASM_OBJ=\
	wasm.o\
	wasm.res\
	werr.o\
	wrelease.o\
	$(STASM_OBJ)

THATCH_OBJ=\
	libthatch.o\
	err.o\
	release.o\
	thatch.o\
	$(STASM_OBJ)

FDET_OBJ=\
	fdet.o\
	err.o\
	release.o\
	$(STASM_OBJ)

MARKI_OBJ=\
	imequalize.o\
	marki.o\
	marki.res\
	werr.o\
	wrelease.o\
	$(STASM_OBJ)

# using local copies of DLLs is a brute force way of avoiding DLL path problems

cv210.dll:
	cp $(OPENCV_HOME)/bin/cv210.dll .

cxcore210.dll:
	cp $(OPENCV_HOME)/bin/cxcore210.dll .

highgui210.dll:
	cp $(OPENCV_HOME)/bin/highgui210.dll .

libgsl.dll:
	cp ../gsl/dll/libgsl.dll .

libgslcblas.dll:
	cp ../gsl/dll/libgslcblas.dll .

jpeg62.dll:
	cp ../jpeg/dll/jpeg62.dll .

stasm.exe: $(STASM_MAIN_OBJ) $(LIBS) $(OPENCV_DLLS)
	$(LINK) -o stasm.exe $(LFLAGS) $(STASM_MAIN_OBJ) $(LIBS)

minimal.exe: $(MINIMAL_OBJ) $(LIBS) $(OPENCV_DLLS)
	$(LINK) -o minimal.exe $(LFLAGS) $(MINIMAL_OBJ) $(LIBS)

tasm.exe: $(TASM_OBJ) $(LIBS) $(OPENCV_DLLS)
	$(LINK) -o tasm.exe $(LFLAGS) $(TASM_OBJ) $(LIBS)

fdet.exe: $(FDET_OBJ) $(LIBS) $(OPENCV_DLLS)
	$(LINK) -o fdet.exe $(LFLAGS) $(FDET_OBJ) $(LIBS)

thatch.exe: $(THATCH_OBJ) $(LIBS) $(OPENCV_DLLS)
	$(LINK) -o thatch.exe $(LFLAGS) $(THATCH_OBJ) $(LIBS)

mdiff.exe: mdiff.o
	$(LINK) -o mdiff.exe $(CFLAGS) mdiff.o

../stasm/stasm.hpp:\
  ../image/image.hpp\
  ../image/imfile.hpp\
  ../image/imwrite.hpp\
  ../image/imgiven.hpp\
  ../image/imutil.hpp\
  ../image/jpegutil.hpp\
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
  ../stasm/me17s.hpp\
  ../stasm/prof.hpp\
  ../stasm/readasm.hpp\
  ../stasm/readconf.hpp\
  ../stasm/tab.hpp\
  ../stasm/rowley.hpp\
  ../stasm/rowleyhand.hpp\
  ../stasm/shapefile.hpp\
  ../stasm/shapemodel.hpp\
  ../stasm/sparsemat.hpp\
  ../stasm/startshape.hpp\
  ../stasm/safe_alloc.hpp\
  ../stasm/util.hpp\
  ../stasm/violajones.hpp\
  ../stasm/vjhand.hpp\
  ../tasm/eyesynth.hpp\
  ../tasm/mrand.hpp\
  ../tasm/mrand.hpp\
  ../tasm/proftrain.hpp\
  ../tasm/tasm.hpp\
  ../tasm/detav.hpp\
  ../tasm/tasmshapes.hpp\
  ../tasm/tcovar.hpp
	touch ../stasm/stasm.hpp

asmsearch.o: ../stasm/asmsearch.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

atface.o: ../stasm/atface.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

err.o: ../stasm/err.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

imshape.o: ../stasm/imshape.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

initasm.o: ../stasm/initasm.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

landmarks.o: ../stasm/landmarks.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

main.o: ../stasm/main.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

prof.o: ../stasm/prof.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

readasm.o: ../stasm/readasm.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

readconf.o: ../stasm/readconf.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

release.o: ../stasm/release.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

rowley.o: ../stasm/rowley.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

rowleyhand.o: ../stasm/rowleyhand.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

shapefile.o: ../stasm/shapefile.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

shapemodel.o: ../stasm/shapemodel.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

sparsemat.o: ../stasm/sparsemat.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

startshape.o: ../stasm/startshape.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

safe_alloc.o: ../stasm/safe_alloc.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

stasm.o: ../stasm/stasm.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

tab.o: ../stasm/tab.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

util.o: ../stasm/util.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

violajones.o: ../stasm/violajones.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

vjhand.o: ../stasm/vjhand.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

werr.o: ../stasm/werr.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

wrelease.o: ../stasm/wrelease.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

fdet.o: ../tools/fdet.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

thatch.o: ../winapps/thatch/thatch.cpp ../stasm/stasm.hpp ../winapps/thatch/libthatch.hpp
	$(CC) -c $< $(CFLAGS)

libthatch.o: ../winapps/thatch/libthatch.cpp ../stasm/stasm.hpp ../winapps/thatch/libthatch.hpp
	$(CC) -c $< $(CFLAGS)

wasm.o: ../wasm/wasm.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

marki.o: ../marki/marki.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

minimal.o: ../minimal/minimal.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

eyesynth.o: ../tasm/eyesynth.cpp ../stasm/stasm.hpp ../tasm/tasm.hpp
	$(CC) -c $< $(CFLAGS)

mat.o: ../mat/mat.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

mchol.o: ../mat/mchol.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

matview.o: ../mat/matview.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

matvec.o: ../mat/matvec.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

imequalize.o: ../image/imequalize.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

imfile.o: ../image/imfile.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

imwrite.o: ../image/imwrite.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

imgiven.o: ../image/imgiven.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

imutil.o: ../image/imutil.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

rgbimutil.o: ../image/rgbimutil.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

ezfont.o: ../image/ezfont.c ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

jpegutil.o: ../image/jpegutil.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

forward.o: ../rowley/forward.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

follow.o: ../rowley/follow.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

imu.o: ../rowley/imu.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

find.o: ../rowley/find.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

initnet.o: ../rowley/initnet.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

search.o: ../rowley/search.cpp ../stasm/stasm.hpp ../rowley/list.hpp
	$(CC) -c $< $(CFLAGS)

wrbmp.o: ../jpeg/wrbmp.c ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

regex.o: ../regex/regex.c ../regex/regex.h ../stasm/stasm.hpp
	gcc -c $< $(REGEX_DEFS) $(CFLAGS)

proftrain.o: ../tasm/proftrain.cpp ../stasm/stasm.hpp ../tasm/tasm.hpp
	$(CC) -c $< $(CFLAGS) ../tasm/proftrain.cpp

tasm.o: ../tasm/tasm.cpp ../stasm/stasm.hpp ../tasm/tasm.hpp ../tasm/tasmshapes.hpp ../tasm/detav.hpp
	$(CC) -c $< $(CFLAGS)

detav.o: ../tasm/detav.cpp ../stasm/stasm.hpp ../tasm/detav.hpp
	$(CC) -c $< $(CFLAGS)

tasmshapes.o: ../tasm/tasmshapes.cpp ../stasm/stasm.hpp ../tasm/tasm.hpp
	$(CC) -c $< $(CFLAGS)

tcovar.o: ../tasm/tcovar.cpp ../stasm/stasm.hpp
	$(CC) -c $< $(CFLAGS)

list.o: ../rowley/list.cpp ../rowley/list.hpp
	$(CC) -c $< $(CFLAGS)

tclHash.o: ../rowley/tclHash.c ../rowley/tclHash.h
	$(CC) -c $< $(CFLAGS)

mrand.o: ../tasm/mrand.cpp ../tasm/mrand.hpp
	$(CC) -c $< $(CFLAGS)

mdiff.o: ../mdiff/mdiff.cpp
	$(CC) -o mdiff.o -c ../mdiff/mdiff.cpp $(CFLAGS)

