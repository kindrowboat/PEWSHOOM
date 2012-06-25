// $prof.hpp 3.0 milbo$ routines for ASM profiles

#if !defined(proftrain_hpp)
#define proftrain_hpp

int nGenElemsPerSubProf(unsigned ProfSpec);

void InitGradsIfNeededForModelGeneration (Mat &Grads,   // out:
            const Image &Img,   // in: Img already scaled to this pyr lev
            int iLev, int nPoints);

unsigned GetGenProfSpec(int iLev, int iPoint);

#endif // proftrain_hpp
