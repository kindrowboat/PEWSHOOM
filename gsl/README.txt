README.txt for stasm/GSL
------------------------

These are the minimal set of files needed for the GNU Scientific
Library for stasm.  Stasm uses GSL for matrix and linear algebra, via
the files in stasm/mat.  README-gsl-1.4.txt is the original README
that came with the GSL software.

Warning: if you have decent versions of the GSL and GSL BLAS libraries
then use those rather than these.  See gsl\lib_ms\README.txt.  Stasm uses
the same gsl/config.h for the Microsoft C build and the mingw gcc
build --- that's not right, but doesn't seem to affect stasm.
