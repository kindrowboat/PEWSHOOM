// $tcovar.cpp 3.0 milbo$ routines for dealing with "trimmed" covariance matrices
//
// Some notes on the parameter nTrimCovar
//
// nTrimCovar is used during training and only for 2d profiles.
//
// If nonzero, trim the covariance matrix so that each profile element
// can be influenced only by other profile elements that are no more than
// nTrimCovar-1 pixels away.
//
// Make nTrimCovar=1 to use only only the diagonal elems (i.e variances,
// no covariances).
//
// Make nTrimCovar=0 to use the entire covariance matrix as is (no
// trimmed distance processing)
//
// Max nTrimCovar is 1+(nProfWidth2d-1)/2
//
// The main reason we use nTrimCovar is for efficiency -- it reduces the
// number of operations needed to calculate xAx.
//
//-----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// A copy of the GNU General Public License is available at
// http://www.r-project.org/Licenses/
//-----------------------------------------------------------------------------

#include "stasm.hpp"

//-----------------------------------------------------------------------------
// Trim the covariance matrix Covar so that each profile element can be
// influenced only by other profile elements that are no more than nDist
// pixels away.  We assume Covar was created from a two dimensional profile,
// giving a block form covariance matrix.
//
// If you want to use all elements then set nDist=nProfWidth2d, and Covar
// will be unchanged.  Although then it is a waste of time to call this
// routine.
//
// Example: if nProfWidth2d=11 and nDist=5, then the first part of
// the covar matrix becomes (where u means used, . means unused i.e. set to 0):
// Notes: the lower triangle of blocks isn't filled in
//        there are 6=5+1 in total blocks used for each level
//
//             Block0     Block1     Block2     Block3     Block4     Block5...
//  Block0  .: uuuuuu.....uuuuuu.....uuuuuu.....uuuuuu.....uuuuuu.....uuuuuu......
//          1: uuuuuuu....uuuuuuu....uuuuuuu....uuuuuuu....uuuuuuu....uuuuuuu.....
//          2: uuuuuuuu...uuuuuuuu...uuuuuuuu...uuuuuuuu...uuuuuuuu...uuuuuuuu....
//          3: uuuuuuuuu..uuuuuuuuu..uuuuuuuuu..uuuuuuuuu..uuuuuuuuu..uuuuuuuuu...
//          4: uuuuuuuuuu.uuuuuuuuuu.uuuuuuuuuu.uuuuuuuuuu.uuuuuuuuuu.uuuuuuuuuu..
//          5: uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu.
//          6: .uuuuuuuuuu.uuuuuuuuuu.uuuuuuuuuu.uuuuuuuuuu.uuuuuuuuuu.uuuuuuuuuu.
//          7: ..uuuuuuuuu..uuuuuuuuu..uuuuuuuuu..uuuuuuuuu..uuuuuuuuu..uuuuuuuuu.
//          8: ...uuuuuuuu...uuuuuuuu...uuuuuuuu...uuuuuuuu...uuuuuuuu...uuuuuuuu.
//          9: ....uuuuuuu....uuuuuuu....uuuuuuu....uuuuuuu....uuuuuuu....uuuuuuu.
//         10: .....uuuuuu.....uuuuuu.....uuuuuu.....uuuuuu.....uuuuuu.....uuuuuu.
//  Block1 11: ...........uuuuuu.....uuuuuu.....uuuuuu.....uuuuuu.....uuuuuu.....u
//         12: ...........uuuuuuu....uuuuuuu....uuuuuuu....uuuuuuu....uuuuuuu....u
//         13: ...........uuuuuuuu...uuuuuuuu...uuuuuuuu...uuuuuuuu...uuuuuuuu...u
//         14: ...........uuuuuuuuu..uuuuuuuuu..uuuuuuuuu..uuuuuuuuu..uuuuuuuuu..u
//         15: ...........uuuuuuuuuu.uuuuuuuuuu.uuuuuuuuuu.uuuuuuuuuu.uuuuuuuuuu.u
//         16: ...........uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu
//         17: ............uuuuuuuuuu.uuuuuuuuuu.uuuuuuuuuu.uuuuuuuuuu.uuuuuuuuuu.
//         18: .............uuuuuuuuu..uuuuuuuuu..uuuuuuuuu..uuuuuuuuu..uuuuuuuuu.
//         19: ..............uuuuuuuu...uuuuuuuu...uuuuuuuu...uuuuuuuu...uuuuuuuu.
//         20: ...............uuuuuuu....uuuuuuu....uuuuuuu....uuuuuuu....uuuuuuu.
//         21: ................uuuuuu.....uuuuuu.....uuuuuu.....uuuuuu.....uuuuuu.
//  Block2 22: ......................uuuuuu.....uuuuuu.....uuuuuu.....uuuuuu.....u
//         23: ......................uuuuuuu....uuuuuuu....uuuuuuu....uuuuuuu....u
//         24: ......................uuuuuuuu...uuuuuuuu...uuuuuuuu...uuuuuuuu...u
//         25: ......................uuuuuuuuu..uuuuuuuuu..uuuuuuuuu..uuuuuuuuu..u
// continues...

static void TrimCovar (Mat &Covar, int nTrimCovar, unsigned ProfSpec)
{
ASSERT(IS_2D(ProfSpec));
ASSERT(nTrimCovar > 0);

int width = nGetProfWidth(Covar.ncols(), ProfSpec);
// so parameter nTrimCovar=1 corresponds only to diag elems
const int nTrimCovar1 = nTrimCovar - 1;
ASSERT(nTrimCovar1 <= (width-1)/ 2);

Mat NewCovar(Covar.nrows(), Covar.ncols());
NewCovar.fill(0);

// upper triangle blocks (each block is width x width)

for (int iyBlock = 0; iyBlock < width; iyBlock++)
    for (int ixBlock = iyBlock+1; ixBlock < width; ixBlock++)
        if (ABS(ixBlock - iyBlock) <= nTrimCovar1)
            {
            const int iyMin = iyBlock * width;
            const int iyMax = iyMin + width;
            for (int iy = iyMin; iy < iyMax; iy++)
                {
                const int ixMin = MAX(0, iy % width - nTrimCovar1) +
                                  ixBlock * width;

                const int ixMax = MIN(width-1, iy % width + nTrimCovar1) +
                                  ixBlock * width;

                for (int ix = ixMin; ix <= ixMax; ix++)
                    NewCovar(iy, ix) = Covar(iy, ix);
                }
            }

// diagonal blocks (done separately for efficiency -- reduces the number
// of compares in inner loops)

for (int iBlock = 0; iBlock < width; iBlock++)
    {
    const int iyMin = iBlock * width;
    const int iyMax = iyMin + width;
    for (int iy = iyMin; iy < iyMax; iy++)
        {
        const int ixMin = MAX(0, iy % width - nTrimCovar1) + iBlock * width;
        const int ixMax = MIN(width-1, iy % width + nTrimCovar1) + iBlock * width;
        for (int ix = ixMin; ix <= ixMax; ix++)
            NewCovar(iy, ix) = Covar(iy, ix);
        }
    }
Covar = NewCovar;

// make symmetrical again (so we can check it later for pos definiteness)

const int nRows = Covar.nrows();

for (int i = 0; i < nRows; i++)
    for (int j = 0; j < i; j++)
        Covar(i,j) = Covar(j,i);
}

//-----------------------------------------------------------------------------
// This forces A to be positive definite by doing the following:
//   Do spectral decomposition: A = Q D Q'
//      where D is a diagonal matrix of (sorted) eig values
//   Set small or negative elements of D to a small value
//   Reconstruct A using the new D in A = Q D Q'
//
// TODO there are probably better ways of forcing positive
// definiteness.  See for example Gentle and nearCor.r in
// the R Matrix package:
// http://cran.r-project.org/web/packages/Matrix/index.html

static void ForcePosDef (Mat &A, int iter) // io:
{
Vec EigVals(A.nrows());
Vec EigVecs = GetEigsForSymMat(A, EigVals); // returns sorted eigs
Mat Diag(A.nrows(), A.nrows());             // all zeroes to start off with

double Min = EigVals.minElem();

// if (Min > 0)
//     Err("ForcePosDef called on a matrix that is already positive definite");

#if 1 // new code
    Min = iter * -Min;
#else // old code (masm version 1.6 and earlier)
    Min = iter * A.trace() / (A.nrows() * 1e3);
#endif

for (unsigned i = 0; i < EigVals.nelems(); i++)
    Diag(i, i) = MAX(EigVals(i), Min);

A = EigVecs * Diag * EigVecs.t();
}

//-----------------------------------------------------------------------------
// Trim the matrix and force it to be positive definite
// Returns true if the covar matrix is pos def

void IterativeTrimMat (Mat &Covar, int nTrimCovar, unsigned ProfSpec)
{
int iter = 0;
bool fPosDefinite;

TrimCovar(Covar, nTrimCovar, ProfSpec);

while (!(fPosDefinite = fPosDef(Covar)) && ++iter <= 10) // assignment is intentional
    {
    // this check is redundant because ForceTrimMat does the same
    // check, but it allows us to give a more specific error message here

    if (!fIsMatrixSymmetric(Covar, Covar(0,0) / 1e5))
        Err("covar matrix %dx%d not symmetric\n"
            "       nMaxShapes in .conf file is too small?",
            Covar.nrows(), Covar.ncols());

    ForcePosDef(Covar, iter);       // force matrix positive def
    TrimCovar(Covar, nTrimCovar, ProfSpec);
    }
if (!fPosDefinite)
    Warn("could not trim mat");

// The covar matrix now has lots of 0 elements and processing
// will be faster if we store it as a sparse matrix.
// This also deletes any very small elems TODO why is this needed?
// AvTrace is the mean val of the diagonal elements.

double AvTrace = Covar.trace() / Covar.ncols();
ConvertSymmetricMatToSparseFormat(Covar, AvTrace / 1e6);
}

//-----------------------------------------------------------------------------
// Convert the given matrix, which must be a symmetric matrix, to sparse format.
// We only store entries for the upper triangle, that's why the matrix
// should be symmetric.
//
// The sparse format is:
//
//      iEntry  Col0  Col1  Col2
//      0       nrows ncols MagicNbr
//      1..n    iRow  iCol  value
//
// The diagonal entries are first, then the rest of the entries.
// All diagonal entries are always stored even if they are 0.
//
// When building the sparse matrix, the lower left triangle elements
// of A are ignored because it's assumed A is symmetrical

void ConvertSymmetricMatToSparseFormat (Mat &A,         // io
                                        double MinVal)  // in: clear any element
                                                        //     smaller than this
{
int nRows = A.nrows(), nCols = A.ncols();
Mat B(nRows * nCols, 3);        // working array

B(0,0) = nRows;
B(0,1) = nCols;
B(0,2) = SPARSE_SYMMETRIC;

int nEntries = 1;
ASSERT(nRows == nCols);

// Diag entries.
// We always include all diag entries, even if they are 0.
// CopyMatToSparseMat assumes that all diag entries are present.

for (int iDiag = 0; iDiag < nRows; iDiag++)
    {
    B(nEntries, 0) = iDiag;
    B(nEntries, 1) = iDiag;
    B(nEntries, 2) = A(iDiag, iDiag);
    nEntries++;
    }
// upper triangle entries

for (int iRow = 0; iRow < nRows; iRow++)
    // start at iRow+1 to get upper tri only
    for (int iCol = iRow+1; iCol < nCols; iCol++)
        if (fabs(A(iRow, iCol)) > MinVal)
            {
            B(nEntries, 0) = iRow;
            B(nEntries, 1) = iCol;
            B(nEntries, 2) = A(iRow, iCol);
            nEntries++;
            }

// copy working array B back into A

A.dim(nEntries, 3);
for (int iEntry = 0; iEntry < nEntries; iEntry++)
    {
    A(iEntry, 0) = B(iEntry, 0);
    A(iEntry, 1) = B(iEntry, 1);
    A(iEntry, 2) = B(iEntry, 2);
    }
}
