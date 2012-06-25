# convert-shape-to-opencv-csv.awk: convert a Stasm shape file to a CSV file with OpenCV coordinates
# Usage: awk -f ../tools/convert-shape-to-opencv-csv.awk ../data/muct76.shape >muct76-opencv.csv

/^ss/           { width = 480;  height = 640; # adjust these for the images you are processing!
                  printf("# %s\n#\n", $0);
                  printf("# format: name, tag, npoints, x0, y0, x1, y1, x2, y2, x3, y3, ...\n#\n") }

/^#/            { print }

/^"[0-9a-fA-F]/ { printf("\"%s,%s\",", $2, $1) }        # e.g. "0000 i000qa-fn"

/^\{ .. 2/      { printf("%d", $2) }                    # e.g. { 76 2

/^\{ 1 8 /      { printf("4,0,0,0,0,0,0,0,0") }         # e.g. { 1 8 ...


/^[-0-9]/       { if($1==0 && $2==0) {                  # x y position of a landmark, e.g. -32, 9
                      # special handling for unavailable points, indicated by 0,0
                      x = 0;
                      y = 0
                  } else {
                      x = $1 + width/2
                      y = height/2 - $2
                      # jitter if now incorrectly marked as unavailable
                      if(x == 0 && y == 0)
                          x = 0.1
                  }
                  printf(",%g,%g", x, y)
                }

/^\}/           { printf("\n") }
