README.txt for stasm.gsl/lib_ms
-------------------------------

These are GSL libraries built under Microsoft C version 6.0.

If you have decent versions of the GSL and GSL BLAS libraries then use
those rather than these.

Warning: These libraries were built by hacking GSL 1.4 and
incorporating matrix and linear algebra files from GSL 1.6.  They were
built in 2005, before decent GSL ports to the Microsoft compiler were
freely available.  Not all functions in GSL are in the in the "mini_"
libraries -- but enough to build masm and stasm.  These libs should
really be updated to an "official" build of GSL under Microsoft C.

Here is a list of the files linked into the mini_ libraries:
    blas.lib
    block.lib
    eigen.lib
    err.lib
    linalg1_6.lib
    matrix1_6.lib
    permutation.lib
    sys.lib
    vector.lib
