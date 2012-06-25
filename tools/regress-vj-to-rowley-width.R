# $regress-vj-to-rowley-width.R 1.7d milbo$
#
# Derive an equation predicting the rowley width as a
# function of the viola jones width. The results are
# used in FindEyesGivenVjFace() in rowley/find.cpp.

# 68.R is 68.shape as an R file
d <- read.table("../data/68.R", header=TRUE, stringsAsFactors=FALSE)
d <- d[d$vj.w != 0 & d$row.w != 0,] # drop ininitialized widths
vj.w <- d$vj.w
row.w <- d$row.w
a <- lm(row.w ~ vj.w)
cat("rowley width as predicted by vj width:\n");
print(a$coefficients)

vj.y = d$vj.y
row.y = d$row.y

cat("\nmean((vj.y - row.y) / row.w) ", mean((vj.y - row.y) / row.w), "\n");

# some plotting, not strictly necessary

par(mfrow=c(2,2))
plot(vj.w, row.w)
library(earth)
plotmo(a, do.par=0, main="linear model", col.resp=2)
e <- earth(row.w ~ vj.w)
plotmo(e, do.par=0, main="MARS model", col.resp=2)
plot(density((vj.y - row.y) / row.w), main="(vj.y - row.y) / row.w")
