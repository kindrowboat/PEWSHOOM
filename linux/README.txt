How to build Stasm under Linux
------------------------------

These are Ubuntu instructions, modify them if you are not on Ubuntu.
We used Ubuntu 9.04

1) Get the libraries needed by Stasm using "sudo apt-get install package_name":
      (a) gsl     (we used libgs10-dev)
      (b) libjpeg (we used libjpeg62 6b-14)
      (c) opencv  (we used version 2.1.0, this version number is probably important)

   For details on these libraries:
      OpenCV                  http://opencv.willowgarage.com/wiki
      Gnu Scientific Library  www.gnu.org/software/gsl
      Independent JPEG group  www.ijg.org

2) Run "make" in the linux directory (we used gcc 4.3.3 and GNU Make 3.81).

3) Test the build by running "make test".

Thanks to the following people for help porting Stasm to Linux:

Brad Yearwood
Pierre Moreels
Tamas Lengyel
Ryan Lei
