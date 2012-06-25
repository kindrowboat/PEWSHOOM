# mirror-facedet.awk: print the faceded shapes in the input shape
#                     file but with mirrored x coords
#
# Usage: awk -f mirror-facedet.awk ../data/muct-ade.shape >tempfile
#
# After running this, change the filenames (to make the 2nd
# character "r") by hand e.g. with EMACS keyboard macros.
# You must also manually change -9999 to 9999.
#-----------------------------------------------------------------------------

/^"1/      { print; }
/^"00001/  { print; }
/^"2/      { print; }
/^"00002/  { print; }
/^}/       { print; }
/^#/       { print; }

/^\{ 1 8 / { printf("{ 1 8 %d %d %d %d %d %d %d %d\n", 
                    -$4, $5, $6, $7, -$10, $11, -$8, $9); }
