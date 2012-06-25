How to build stasm using GCC in a MINGW environment
---------------------------------------------------

The simplest way is to run the mingw make and fix any problems
that come up.  There will be a few issues because some of the
paths in the makefile must be changed for your environment.

Alternatively, here are detailed steps:

1. Make sure you have the necessary MINGW GNU tools on your path as follows:

   1a. Enter: gcc --version
       You should get this message: sgcc (GCC) 4.2.1-sjlj (mingw32-2) ...
       A similar gcc version should also be ok

   1b. Enter: make --version
       You should get this message: GNU Make version 3.79.1, ...

   I installed my mingw from the following site (and it is a very
   easy way of installing mingw):
   www.murdoch-sutherland.com/Rtools/installer.html

2. Make sure you have OpenCV 2.1 installed into C:\OpenCV2.1
   directory.  Google for OpenCV and use the self installing .exe file.
   The last time I looked it was available here:
   http://opencv.willowgarage.com/wiki

2. Change into the "stasm/mingw" directory

3. Modify the makefile by changing the paths at the top of the makefile
   to your paths

4. Do a "make clean" to start from a clean slate.

5. To build the executables, execute "make"
   and you should end up with stasm.exe and its friends.

6. To test the build, execute "make test".
   The test set is not as complete as for the Visual C builds
   because numerical issues cause small differences
   and I did not want to duplicate all the reference test files.
   However, the mean fits are just as good.
