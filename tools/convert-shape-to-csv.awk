# convert-shape-to-csv.awk: convert a Stasm shape file to a CSV file
# Usage: awk -f ../tools/convert-shape-to-csv.awk ../data/muct76.shape >muct76.csv

/^ss/           { printf("# %s\n#\n", $0);
                  printf("# format: name, tag, npoints, x0, y0, x1, y1, x2, y2, x3, y3, ...\n#\n") }

/^#/            { print }

/^"[0-9a-fA-F]/ { printf("\"%s,%s\",", $2, $1) }                                          # e.g. "0000 i000qa-fn"

/^\{ .. 2/      { printf("%d", $2) }                                                      # e.g. { 76 2

/^\{ 1 8 /      { printf("4,%g,%g,%g,%g,%g,%g,%g,%g", $4, $5, $6, $7, $8, $9, $10, $11) } # e.g. { 1 8 ...

/^[-0-9]/       { printf(",%g,%g", $1, $2) }                                              # e.g. -39 -28

/^\}/           { printf("\n") }
