# mirror-muct68.awk: print the MUCT shapes in the input shape
#                    file but with mirrored x coords
#
# Usage: awk -f ../tools/mirror-muct68.awk x.shape >xr.shape
#
# After running this, change the filenames (to make the 2nd
# character "r") by hand e.g. with EMACS keyboard macros.
#-----------------------------------------------------------------------------

# by default ignore shapes

/^"/    { use = 0; }

# the "^0...0... i" matches MUCT shapes but not face det shapes
# the ^r is so we skip existing mirrored shapes, if any

/^"0...0... i[^r]/  { use = 1; ipoint = 0; printf("%s\n", $0); }

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

function myround(x) { # round to one decimal place
    i = int(x * 10)
    return i / 10
}

function myprint(i) {
    x = 0; y = 0;
    if (shapex[i] != 0 || shapey[i] != 0) { # not an unused point?
        x = myround(-shapex[i]-1);
        y = myround(shapey[i]);
        if (x == 0 && y == 0) # falsely marked as an unused point after conversion?
            x = 0.1           # adjust so point is no longer marked as unused
    }
    printf("%g %g\n", x, y)
}

/^\}/   {

        if (use) {
            if (ipoint != 68)
                printf("Error: ipoint != 68\n");
            printf("{ 68 2\n"); # index oldname
            myprint(14);        #  0 14_RTemple
            myprint(13);        #  1 13_RJaw1
            myprint(12);        #  2 12_RJaw2
            myprint(11);        #  3 11_RJaw3
            myprint(10);        #  4 10_RJaw4
            myprint(09);        #  5 09_RJaw5
            myprint(08);        #  6 08_RJaw6
            myprint(07);        #  7 07_TipOfChin
            myprint(06);        #  8 06_LJaw6
            myprint(05);        #  9 05_LJaw5
            myprint(04);        # 10 04_LJaw4
            myprint(03);        # 11 03_LJaw3
            myprint(02);        # 12 02_LJaw2
            myprint(01);        # 13 01_LJaw1
            myprint(00);        # 14 00_LTemple
            myprint(21);        # 15 21_LOuterEyeBrow
            myprint(22);        # 16 22_LOuterTopEyeBrow
            myprint(23);        # 17 23_LInnerTopEyeBrow
            myprint(24);        # 18 24_LInnerEyeBrow
            myprint(25);        # 19 25_Point25
            myprint(26);        # 20 26_Point26
            myprint(15);        # 21 15_ROuterEyeBrow
            myprint(16);        # 22 16_ROuterTopEyeBrow
            myprint(17);        # 23 17_RInnerTopEyeBrow
            myprint(18);        # 24 18_RInnerEyeBrow
            myprint(19);        # 25 19_Point19
            myprint(20);        # 26 20_Point20
            myprint(32);        # 27 32_REyeOuter
            myprint(33);        # 28 33_REyeTop
            myprint(34);        # 29 34_REyeInner
            myprint(35);        # 30 35_REyeBottom
            myprint(36);        # 31 36_REye
            myprint(27);        # 32 27_LEyeOuter
            myprint(28);        # 33 28_LEyeTop
            myprint(29);        # 34 29_LEyeInner
            myprint(30);        # 35 30_LEyeBottom
            myprint(31);        # 36 31_LEye
            myprint(45);        # 37 45_RNoseTop
            myprint(44);        # 38 44_MRNoseMid
            myprint(43);        # 39 43_RNoseBot0
            myprint(42);        # 40 42_RNoseBot1
            myprint(41);        # 41 41_Nosebase
            myprint(40);        # 42 40_LNoseBot1
            myprint(39);        # 43 39_LNoseBot0
            myprint(38);        # 44 38_LNoseMid
            myprint(37);        # 45 37_LNoseTop
            myprint(47);        # 46 47_RNostril
            myprint(46);        # 47 46_LNostril
            myprint(54);        # 48 54_RMouthCorner
            myprint(53);        # 49 53_Mouth53
            myprint(52);        # 50 52_Mouth52
            myprint(51);        # 51 51_MouthTopOfTopLip
            myprint(50);        # 52 50_Mouth50
            myprint(49);        # 53 49_Mouth49
            myprint(48);        # 54 48_LMouthCorner
            myprint(59);        # 55 59_Mouth59
            myprint(58);        # 56 58_Mouth58
            myprint(57);        # 57 57_MouthBotOfBotLip
            myprint(56);        # 58 56_Mouth56
            myprint(55);        # 59 55_Mouth55
            myprint(62);        # 60 62_Mouth62
            myprint(61);        # 61 61_Mouth61
            myprint(60);        # 62 60_Mouth60
            myprint(65);        # 63 65_Mouth65
            myprint(64);        # 64 64_Mouth64
            myprint(63);        # 65 63_Mouth63
            myprint(66);        # 66 66_MouthBotOfTopLip
            myprint(67);        # 67 67_NoseTip

            printf("}\n");
        }
    }
