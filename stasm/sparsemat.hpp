// $sparsemat.hpp 3.0 milbo$ routines for dealing with sparse matrices

#if !defined(sparsemat_hpp)
#define sparsemat_hpp

// Row layout is:
//      1st row: nrows, ncols, SPARSE_SYMMETRIC
//      N following rows: all diag entries
//      remaining rows: upper triangle entries
//
// Note that the lower triangular entries aren't stored (that's
// unneeded because the matrix is symmetric).
//
// In SPARSE_ELEM, we use shorts and floats rather than
// ints and doubles for a small speed improvement
// (TODO does this actually improve speed?)

typedef struct SPARSE_ELEM  // an element in a sparse array
    {
    short   iRow, iCol;
    float   Val;
    }
SPARSE_ELEM;

// special value for A(0,2) to mark an array as sparse symmetric

static const double SPARSE_SYMMETRIC = -9999;

typedef vector<SPARSE_ELEM> SparseMat;

double Sparse_xAx(const Vec &x, const SparseMat &A);
void CopyMatToSparseMat(SparseMat &S, const Mat &M);
bool fSparseMat(const Mat &A);

#endif // sparsemat_hpp
