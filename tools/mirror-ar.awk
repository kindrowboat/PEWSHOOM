# mirror-ar.awk: print the AR shapes in the input shape
#                file but with mirrored x coords
#
# After running this, change the filenames (to make the 2nd
# character "r") by hand e.g. with EMACS keyboard macros.
#-----------------------------------------------------------------------------

# by default ignore shapes

/^"/    { use = 0; }

# the "^0... a" matches AR shapes but not face det shapes
# the ^r is so we skip existing mirrored shapes, if any

/^"0... a[^r]/  { use = 1; ipoint = 0; printf("%s\n", $0); }

# following gnarly pattern matches x y coords for a shape i.e. number number
# with optional white space around the numbers

/^[ \t]*[-0-9][.0-9]*[ \t]+[-0-9][.0-9]*[ \t]*$/    {

        if (use) {
                shapex[ipoint] = $1;
                shapey[ipoint] = $2;
                ipoint++;
        }
    }

# closing curly brace so print the points in mirrored order
# note the minus before each shapex i.e. mirror the shape

/^\}/   {

        if (use) {
            if (ipoint != 22)
                printf("Error: ipoint != 22\n");
            printf("{ 22 2\n");                               # index oldname
            printf("%0.1f %0.1f\n", -shapex[ 1], shapey[ 1]); #  0    REye
            printf("%0.1f %0.1f\n", -shapex[ 0], shapey[ 0]); #  1    LEye
            printf("%0.1f %0.1f\n", -shapex[ 3], shapey[ 3]); #  2    RMouthCorner
            printf("%0.1f %0.1f\n", -shapex[ 2], shapey[ 2]); #  3    LMouthCorner
            printf("%0.1f %0.1f\n", -shapex[ 7], shapey[ 7]); #  4    ROuterEyeBrow
            printf("%0.1f %0.1f\n", -shapex[ 6], shapey[ 6]); #  5    RInnerEyeBrow
            printf("%0.1f %0.1f\n", -shapex[ 5], shapey[ 5]); #  6    LInnerEyeBrow
            printf("%0.1f %0.1f\n", -shapex[ 4], shapey[ 4]); #  7    LOuterEyeBrow
            printf("%0.1f %0.1f\n", -shapex[13], shapey[13]); #  8    MRTemple
            printf("%0.1f %0.1f\n", -shapex[12], shapey[12]); #  9    REyeInner
            printf("%0.1f %0.1f\n", -shapex[11], shapey[11]); # 10    REyeOuter
            printf("%0.1f %0.1f\n", -shapex[10], shapey[10]); # 11    LEyeInner
            printf("%0.1f %0.1f\n", -shapex[ 9], shapey[ 9]); # 12    LEyeOuter
            printf("%0.1f %0.1f\n", -shapex[ 8], shapey[ 8]); # 13    MLTemple
            printf("%0.1f %0.1f\n", -shapex[14], shapey[14]); # 14    NoseTip
            printf("%0.1f %0.1f\n", -shapex[16], shapey[16]); # 15    RNostril
            printf("%0.1f %0.1f\n", -shapex[15], shapey[15]); # 16    LNostril
            printf("%0.1f %0.1f\n", -shapex[17], shapey[17]); # 17    MouthTopOfTopLip
            printf("%0.1f %0.1f\n", -shapex[18], shapey[18]); # 18    MouthBotOfBotLip
            printf("%0.1f %0.1f\n", -shapex[19], shapey[19]); # 19    TipOfChin
            printf("%0.1f %0.1f\n", -shapex[21], shapey[21]); # 20    RJaw3
            printf("%0.1f %0.1f\n", -shapex[20], shapey[20]); # 21    LJaw3
            printf("}\n");
        }
    }
