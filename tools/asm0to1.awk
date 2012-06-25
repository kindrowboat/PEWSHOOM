# asm0to1.awk: convert a version 0 to a version 1 ASM file

# the is_asm0 variable allows us to "convert" a version 1 file with no changes

/^ASM0/                 { is_asm0 = 1; printf("ASM1 %s --- converted by asm0to1.awk]\n", $2); next; }
/^Points/               { printf("{\nnPoints %s\nfXm2vts 1\n", $2); next; }
/^Levs/                 { printf("nLevs %s\n", $2); next; }
/^PyramidRatio/         { printf("PyramidRatio %s\n", $2); next; }
/^PyramidReduceMethod/  { printf("nPyramidReduceMethod %s\n", $2); next; }
/^ExplicitPrevNext/     { printf("fExplicitPrevNext %s\n", $2); next; }
/^NormalizedProfLen/    { printf("NormalizedProfLen %s\n", $2); next; }
/^ScaledFaceWidth/      { printf("nStandardFaceWidth %s\n", $2); next; }
/^BilinearRescale/      { printf("fBilinearRescale %s\n", $2); next; }
/^TrimCovar/            { printf("nTrimCovar %s\n", $2); next; }
/^SigmoidScale/         { printf("SigmoidScale %s\n", $2); if (is_asm0) printf("}\n"); next; }
/^# EigVals/            { printf("\"EigVals\"\n"); next; }
/^# EigVecs/            { printf("\"EigVecs\"\n"); next; }
/^# AvShape/            { in_av_shape = 1; printf("\"AvShape\"\n"); next; }
/^# VjAv/               { printf("\"VjAv\"\n"); next; }
/^# RowleyAv/           { printf("\"RowleyAv\"\n"); next; }
/^# Lev .* ProfSpecs/   { printf("\"ProfSpecs Lev %s\"\n", $3); next; }
/^# Covar Lev/          { printf("\"Covar Lev %s Point %s\"\n", $4, $6); next; }
/^# Lev .* Prof /       { printf("\"Lev %s Prof %s\"\n", $3, $5); next; }
/^Lev [0-9] FirstShape/ { printf("Lev %s\n", $2); next; }
/}/                     { if (in_av_shape) # add dummy VjAv and RowleyAv matrices
                                printf("}\n\"VjAv\"\n{ 0 0\n}\n\"RowleyAv\"\n{ 0 0\n}\n");
                          else
                                print
                          in_av_shape = 0;
                          next;
                        }
/./                     { print }
