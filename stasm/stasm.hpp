// $stasm-stasm.hpp 3.0 milbo$ (nearly) all include files for stasm

#if !defined(stasm_all_hpp)
#define stasm_all_hpp

static const char * const STASM_VERSION = "version 3.0";

#if _MSC_VER                        // microsoft
  #ifndef _CRT_SECURE_NO_WARNINGS
     #define _CRT_SECURE_NO_WARNINGS 1
  #endif
  #pragma warning(disable:4996)     // disable non-secure function warnings
  #pragma warning(disable:4786)     // disable long names in STL warnings
#else
  #define stricmp strcasecmp
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>

using namespace std;

#include "safe_alloc.hpp" // safe_alloc.hpp must be first to redefine malloc and alloc
#if _WIN32  // TODO why?
#include "gsl/config.h"   // config.h must be first to build with inline defined etc.
#endif
#include "gsl_nan.h"
#include "gsl_errno.h"
#include "gsl_math.h"
#include "gsl_matrix.h"
#include "gsl_linalg.h"
#include "gsl_eigen.h"
#include "gsl_blas.h"
#include "util.hpp"
#include "err.hpp"
#include "release.hpp"
#include "mat.hpp"
#include "matview.hpp"
#include "mchol.hpp"

using namespace GslMat;

#include "image.hpp"
#include "jpegutil.hpp"
#include "pngutil.hpp"
#include "colors.hpp"
#include "matvec.hpp"
#include "atface.hpp"
#include "landmarks.hpp"
#include "mouth.hpp"
#include "regex.h"
#include "prof.hpp"
#include "sparsemat.hpp"
#include "startshape.hpp"
#include "asmsearch.hpp"
#include "shapemodel.hpp"
#include "imshape.hpp"
#include "imutil.hpp"
#include "rgbimutil.hpp"
#include "regex.h"
#include "imfile.hpp"
#include "imwrite.hpp"
#include "imgiven.hpp"
#include "find.hpp"
#include "rowley.hpp"
#include "rowleyhand.hpp"
#include "violajones.hpp"
#include "vjhand.hpp"
#include "initasm.hpp"
#include "readasm.hpp"
#include "readconf.hpp"
#include "tclHash.h"
#include "forward.hpp"
#include "follow.hpp"
#include "list.hpp"
#include "initnet.hpp"
#include "search.hpp"
#include "tcovar.hpp"
#include "mrand.hpp"
#include "proftrain.hpp"
#include "shapefile.hpp"
#include "me17s.hpp"
#include "tab.hpp"
#include "eyesynth.hpp"

#endif // stasm_all_hpp
