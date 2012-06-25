# $stasm/linux/makefile 3.1 milbo$ linux makefile for stasm
#
# To build: make -f makefile
#
# TODO dependencies aren't all correct -- do make clean to be safe
#
# milbo Petaluma Sep 08

all: mdiff stasm tasm minimal fdet

# modify the following for your environment

LIBS=-lm -lcv -lhighgui -lcvaux -lgsl -lgslcblas -ljpeg

# Try using the following if you are on a mac:
#
# LIBS=-lm -L/opt/local/lib -lcxcore -lcv -lhighgui -lcvaux -lgsl -lgslcblas -ljpeg
#
# -lm:
# -L/opt/local/lib:
# -lcxcore:
# -lcv:
# -lhighgui:
# -lcvaux:
# -lgsl:
# -lgslcblas:
# -ljpeg:

OPENCV_HOME=/home/john/OpenCV-2.1.0

INCL=\
    -I$(OPENCV_HOME)/include/opencv\
    -I../gsl\
    -I../gsl/gsl\
    -I../image\
    -I../jpeg\
    -I../mat\
    -I../rowley\
    -I../stasm\
    -I../tasm

CFLAGS=-O3 -Wall -pedantic $(INCL)
LFLAGS=$(CFLAGS)
CC=g++
LINK=g++

clean:
	rm -f *.o *.log s-*.bmp 1-*.bmp search-results.bmp fdet mdiff minimal stasm tasm 

veryclean: clean
	rm -f *.asm

# For stasm 3.1 I removed most tests here because small numerical
# differences (probably in the OpenCV face finder) cause the reference
# files to be different under Windows and Linux and that causes the
# tests to fail (or look as if they fail).  But the following test is a
# good sanity check, and seems to be consistent across systems.

test: # stasm rowley test with default config files
	@echo === test-stasm-row ==============================================
	./stasm -r -iS ../data/test-image.jpg
	@echo The following mdiff and cmps should give no output:
	./mdiff stasm.log ../tests/mingw-test-row-stasm.log
	cmp 1-row-test-image.bmp ../tests/1-row-test-image.bmp
	cmp s-row-test-image.bmp ../tests/s-row-test-image.bmp
	rm -f stasm.log *-test-image.bmp

mu-68-1d.asm: tasm ../data/tasm-mu-68-1d.conf
	@echo === making mu-68-1d.asm ==========================================
	./tasm -o mu-68-1d.asm ../data/tasm-mu-68-1d.conf

mu-76-2d.asm: tasm ../data/tasm-mu-76-2d.conf
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

FDET_OBJ=\
	fdet.o\
	err.o\
	release.o\
	$(STASM_OBJ)

stasm: $(STASM_MAIN_OBJ) $(LIBS) $(OBJ)
	$(LINK) -o stasm $(LFLAGS) $(STASM_MAIN_OBJ) $(LIBS)

minimal: $(MINIMAL_OBJ) $(LIBS) $(OBJ)
	$(LINK) -o minimal $(LFLAGS) $(MINIMAL_OBJ) $(LIBS)

tasm: $(TASM_OBJ) $(LIBS) $(OBJ)
	$(LINK) -o tasm $(LFLAGS) $(TASM_OBJ) $(LIBS)

fdet: $(FDET_OBJ) $(LIBS) $(OBJ)
	$(LINK) -o fdet $(LFLAGS) $(FDET_OBJ) $(LIBS)

mdiff: mdiff.o
	$(LINK) -o mdiff $(CFLAGS) mdiff.o

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

# regex.o: ../regex/regex.c ../regex/regex.h ../stasm/stasm.hpp
#	gcc -c $< $(REGEX_DEFS) $(CFLAGS)

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
