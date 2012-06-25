# $summary.R 3.0 milbo$
#
# R program to plot summaries from tables produced by stasm with the -t option
# To generate the tab files shown below, use "make muct-paper-tabs".

filenames <- c(
"tab/bioid-12-mutr.tab",
"tab/bioid-12.tab",

# "tab/bioid-12-mutr-easy.tab",
# "tab/bioid-12-easy.tab",
#
# "tab/bioid-12-mutr-hard.tab",
# "tab/bioid-12-hard.tab",
#
# "tab/put-12-mutr.tab",
# "tab/put-12.tab",

# "../data/bioid-12-version2.4.tab",  # results from stasm version 2.4

"" # placeholder so can have a comma terminating final file above
)

xlim <- c(0.0, 0.14)    # x limits on plotted graph

line.cols <-            # colors of plotted lines
    c(1, "slategray", "salmon", 3:20)

do.plot <- TRUE

library(stats)
library(graphics)

# Reproduce the curve in Cristinacce CLM Fig 4c "Feature Detection
# and Tracking with Constrained Local Models".
# The numbers were derived directly from the printed graph in the paper.

plot.clm <- function()
{
    x <- c(.024, .03, .035, .04, .05, .06, .07,  .08,
           .09,  .1,  .11,  .12, .13, .14, .15)

    y <- c(0,     .015, .055,  .135,  .455, .685,  .825, .885,
           .9055, .93,  .9405, .9555, .965, .9655, .975)

    lines(x=x, y=y, lty=2)
}

plot.grid <- function()
{
    for (x in seq(from = xlim[1]+.02, to = xlim[2]-.02, by = .02))
        abline(v=x, lty="solid", col="gray80")
    for (y in seq(from = 0.1, to = .9, by = .1))
        abline(h=y, lty="solid", col="gray80")
    abline(h=.95, lty="solid", col="gray80")
}

if (!identical(filenames[length(filenames)], ""))
    stop("filenames[length(filenames)] != \"\"")
filenames <- filenames[1:(length(filenames)-1)] # drop placeholder
medians <- double(length(filenames))
means <- double(length(filenames))
mouth.medians <- double(length(filenames))
for (ifile in 1:length(filenames)) {
    filename <- filenames[ifile]
    d <- read.table(filename, header=TRUE, stringsAsFactors=FALSE)
    if (identical(colnames(d)[1:4], c("file", "npoints", "start", "final"))) {
        # old format tab file (stasm2.4), convert to new format
        d$npoints = NULL                # delete npoints column
        d$intereye <- rep(0, nrow(d))   # add intereye column
        d$facewidth <- rep(0, nrow(d))  # add facewidth column
    }
    if (!identical(colnames(d)[1:4], c("file", "start", "final", "intereye")))
        stop("unexpected colnames in ", filename)
    is.me17 <- (ncol(d) == 5 || ncol(d) == 22) # 5 for old format, 22 for new format
    stopifnot(is.me17)
    medians[ifile] <- median(d$final)
    means[ifile] <- mean(d$final)
    cat(sprintf("%-32.32s nshapes %4.0d intereye %4.0f width %4.0f med %6.4f mean %6.4f min %6.4f max %6.4f",
        gsub(".tab", "", filename), # TODO gsub(".tab", "", basename(filename)),   # strip dir and ".tab" from filename
        nrow(d), mean(d$intereye), mean(d$facewidth),
        medians[ifile], means[ifile], min(d$final), max(d$final)))
    if (ncol(d) > 5) { # new format
        mouth <- rowMeans(d[,c(3,4,16,17)+5])
        eyes <- rowMeans(d[,c(1,2,9,10,11,12)+5])
        mouth.medians[ifile] <- median(mouth)
        cat(sprintf(" mouth: med %6.4f mean %6.4f", mouth.medians[ifile], mean(mouth)))
        cat(sprintf(" eyes: med %6.4f mean %6.4f", median(eyes), mean(eyes)))
    }
    line.col <- line.cols[ifile]
    if (do.plot) {
        if (ifile == 1) {
            # plot the first graph to set up the graph structure

            par(mar = c(4, 4, 2, 1))             # small margins and text
            par(mgp = c(2, 0.6, 0))              # flatten axis elements
            iorder <- order(d$final, decreasing=TRUE)
            plot(ecdf(d$final), col.p=line.col, col.h=line.col, col.v=line.col,
                xlab = if (is.me17) "me17" else "distance",
                ylab="proportion", xlim=xlim, main="",
                verticals=TRUE, do.points=FALSE,
                xaxs="i", yaxs="i", col.01line = "white")

            plot.grid()
            # abline(h=.5, lty="solid", col="gray")  # median line if you want
            plot.clm()                           # plot CLM points for reference
            # replot over grid
            lines(ecdf(d$final), col.p=line.col, col.h=line.col, col.v=line.col,
                  main="", xlim=xlim, verticals=TRUE, do.points=FALSE)
        } else
            lines(ecdf(d$final), col.p=line.col, col.h=line.col, col.v=line.col,
                  main="", xlim=xlim, verticals=TRUE, do.points=FALSE)
    }
    cat("\n")
}
if (do.plot) {
    # remove the unsightly dotted horizontal lines drawn by
    # plot(cum) at top and bottom of graph

    abline(h=0, col="black")
    abline(h=1, col="black")

    # put legend on graph

    s <- gsub(".tab", "", basename(filenames))   # strip dir and ".tab" from filenames in legend
    ltys = rep(1, length(s))

    s = c(s, "CLM Fig 4.c")                      # append CLM plot details to legend
    ltys = c(ltys[1:(length(s)-1)], 2)
    cols <- c(line.cols[1:(length(s)-1)], 1)

    legend(x=mean(xlim), y=.56, s,
        col=cols, text.col=cols, lty=ltys, bg="White")
}
