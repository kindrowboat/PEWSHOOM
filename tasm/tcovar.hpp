// $tcovar.hpp 3.0 milbo$

#if !defined(tcovar_hpp)
#define tcovar_hpp

void IterativeTrimMat(Mat &Covar, int nTrimCovar, unsigned ProfSpec);
void ConvertSymmetricMatToSparseFormat(Mat &A, double MinVal=0);

#endif // tcovar_hpp
