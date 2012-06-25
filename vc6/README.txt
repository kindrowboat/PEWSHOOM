This directory contains the files to build the Stasm library
under Visual C 6.0.


To build from the command line:

1) First install OpenCV 2.1 
   to C:\OpenCV2.1 (the last time I looked this was available at 
   http://opencv.willowgarage.com/wiki)

2) You will have to make minor edits to "makefile" for you environment.

3) Make sure that the vc6/Debug and vc6/Release directories exist.
   Else you will get the message: 
     fatal error C1083: Cannot open compiler generated file: './Release/xxx'

4) Run make.bat to build the executables.  Run "make test" or make
   "fulltest" to test them.



To build stasm.exe from within the Integrated Development Environment:

1) First install OpenCV 2.1 
   to C:\OpenCV2.1 (the last time I looked this was available at 
   http://opencv.willowgarage.com/wiki)

2) Load the workspace stasm.dsw and rebuild all.
   You may need to change the active project to Release not Debug.
   You may get some messages "could not find the file *.h".
   These appear to be harmless.


Steve Milborrow
Aug 2008
