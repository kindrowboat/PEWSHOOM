README.txt for the Stasm package
--------------------------------

For more details please see the documentation at
www.milbo.users.sonic.net.

These are the source files for Stasm, which is library for
locating features in a face using Active Shape Models.  You
give Stasm an image file (i.e. a JPEG or BMP) and it returns
the image with the facial features marked out on the image
(see tests/s-B1138_20.bmp for an example).  It also
returns the coordinates of the face landmarks in the file
"stasm.log" (see tests/test-stasm.log for an example).

Stasm is designed to work on "passport style" photographs
i.e. on front views of upright faces with neutral
expressions.  It doesn't work well on faces at an angle.

This software is licensed under the Gnu Public License 2,
available at http://www.r-project.org/Licenses.  It uses the
Gnu Scientific Library which also comes with a GPL License
which has implications about how you may legally use the
software.  Also there may be patent issues with use of the
Viola Jones detector.

The stasm executable (and related tools) can be built in the following
environments, each with their own directory.  See the README.txt file
in each directory:
    . minw GCC in a MINGW environment, see mingw/README.txt.
    . Microsoft Visual C 6  (directory is vc6)
    . Microsoft Visual C 9  (directory is vc9)
    . Microsoft Visual C 10 (directory is vc10)
    . Linux GCC             (directory is linux)

Here is a summary of the directories for Stasm:

data/           data files used by Stasm, includes test files
gsl/            gsl includes and prebuilt gsl libraries for Microsoft C.
image/          image class
jpeg/           interface to the JPEG Group's free JPEG software
mat/            matrix classes
mdiff/          utility like unix diff but ignores [TIMES] in square brackets
minimal/        minimal main routine which uses stasm to locate facial features
opencv_example/ simple example using OpenCV and the Stasm DLL
png/            interface to PNG libraries
regex/          regular expression code
rowley/         the Rowley face and eye detector (actually, my version of it)
stasm/          source files for Stasm
tasm/           tool for building new active shape models
tasm-example/   example for Tasm.exe
tools/          miscellaneous tools

tasm-example/ example showing how to use tasm
test/         files for testing stasm (run "make test" to use these)

vc6/          files for building Stasm using Microsoft C++ 6.0
vc9/          files for building Stasm using Microsoft C++ 9.0 2008
vc10/         files for building Stasm using Microsoft C++ 10.0 2010
mingw/        files for building Stasm using gcc in a mingw environment
linux/        files for buildign Stasm under Linux

winapps/casm/    Windows tool for locating face landmarks from webcam
winapps/iland/   Windows tool for interactive face landmarking
winapps/marki/   Windows tool for manually landmarking faces
winapps/thatch/  tools for thatcherizing faces
winapps/wasm/    Windows tool for locating face landmarks in an image file

If you are incorporating the software into an existing application you
will probably want to change functions like Err() in err.cpp.
If you have existing matrix or image libraries you will need to
change the interface to the files in mat/ and image/.

If you already have the GSL for Microsoft C libraries on your system,
it would make sense to use those rather than my copies in gsl/lib_ms
because my copies are rather a hack, and are based on an old version
of GSL.  See the README.txt in the gsl directories.

The term "Stasm" stands for "stacked trimmed ASM" or "stacked two
dimensional ASM" or "Steve's ASM" or anything you want it to stand
for.  Really it's just a tag to refer to the package.

This software is a cleaned up version of the relevant pieces
of my research software.  You will see some artifacts in the
sources that that is the case.

Have fun!  And let me know if you are using the software and if you
have any suggestions or bug fixes.

Stephen Milborrow
www.milbo.users.sonic.net
