// $mrand.cpp 3.0 milbo$ random numbers
// milbo oct 2002, revised aug 2008

#include "stasm.hpp"

static double unif_rand(void);

//-----------------------------------------------------------------------------
// returns an integer in the range 0 to n-1

int Rand (int n)
{
return (int)(n * unif_rand());
}

//-----------------------------------------------------------------------------
// returns a double random number between 0 to n-EPS

double RandDouble (double n)
{
return n * unif_rand();
}

//-----------------------------------------------------------------------------
// Returns a gaussian random number with the given standard deviation
//
// We use the fact that the sum of uniformly distibuted random variables
// approximates a gaussian distribution (by the central limit theorem)

double RandGauss (double StdDev)
{
if (StdDev == 0)    // for efficiency
    return 0;

double r = 0;
for (int i = 0; i < 12; i++)
    r += RandDouble(2) - 1; // add a random number between -1 and +1

return (StdDev * r) / 2;
}

//-----------------------------------------------------------------------------
// Following lifted from code in RNG.c in the R sources.

#define i2_32m1 2.328306437080797e-10       // = 1/(2^32 - 1)

static unsigned int I1 = 1234, I2 = 5678;

void SeedRand (int iSeed)
{
    if (iSeed < 0)
        iSeed = -iSeed;
    if (iSeed == 0)
        iSeed = 1;
    I1 = iSeed;
    I2 = iSeed + 7;  // TODO not too sure what is best here
}

static double fixup(double x)  // ensure 1 is never returned
{
    if(x < 0.0) return 0.5*i2_32m1;
    if((1.0 - x) <= 0.0) return 1.0 - 0.5*i2_32m1;
    return x;
}

// return a random number between 0 inclusive and 1 exclusive

static double unif_rand(void)
{
    // marsaglia multicarry

    I1= 36969*(I1 & 0177777) + (I1>>16);
    I2= 18000*(I2 & 0177777) + (I2>>16);
    return fixup(((I1 << 16)^(I2 & 0177777)) * i2_32m1);
}
