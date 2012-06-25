// $mdiff.cpp 3.1 milbo$ print difference between two files,
//                       ignoring text in either file between []
//
// This prints the FIRST difference of multiple consecutive different lines.
// It then resynchs and looks for further differences.
// Prints up to 10 differences.
//
// I wrote this to do diffs between test results that have different printed
// time results but should be otherwise the same.
// Bracketed time results look like this [Time 2.34]
//
// Warning: this is code written quickly to solve a specific problem -- expect
//          it to be quite messy.
//
// milbo durban dec 05

#if _MSC_VER                        // microsoft
  #define _CRT_SECURE_NO_WARNINGS 1 // disable non-secure function warnings
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

static const int MAX_LEN   = 10000;
static const int MAX_DIFFS = 10;

//-----------------------------------------------------------------------------
static void Err (const char *pArgs, ...)    // args like printf
{
static char s[MAX_LEN];

va_list pArg;
va_start(pArg, pArgs);
vsprintf(s, pArgs, pArg);
va_end(pArg);
printf("%s\n", s);
exit(-1);
}

//-----------------------------------------------------------------------------
// Like fgets but discards \r

char *fgets1 (char *s, int n, FILE *stream)
{
char *p = fgets(s, n, stream);
if (p)
    {
    // discard \r
    int len = strlen(s);
    if (len >= 2 && s[len-2] == '\r')
        {
        s[len-2] = '\n';
        s[len-1] = 0;
        }
    }
return p;
}

//-----------------------------------------------------------------------------
static void PrintErr (int iLine,
                      const char sFile1[], const char sLine1[],
                      const char sFile2[], const char sLine2[])
{
int iLen = strlen(sLine1)-1;
int nLen = strlen(sFile1);
if (strlen(sFile2) > unsigned(nLen))
    nLen = strlen(sFile2);
printf("%*s %5d: %s%s", nLen, sFile1, iLine, sLine1,
    ((iLen < 0 || sLine1[iLen] != '\n')? "\n": ""));
iLen = strlen(sLine2)-1;
printf("%*s %5d: %s%s\n", nLen, sFile2, iLine, sLine2,
    ((iLen < 0 || sLine2[iLen] != '\n')? "\n": ""));
}

//-----------------------------------------------------------------------------
// Like strcmp but skips (i.e. ignores) text between [square brackets]

static int iStrCmpSkipBrackets (char s1[], char s2[])
{
int i, j;
for (i = 0, j = 0; s1[i] && s2[j]; i++, j++)
    {
    if (s1[i] == '[')                       // skip []
        while (s1[i] && s1[i] != ']')
            i++;
    if (s2[j] == '[')
        while (s2[j] && s2[j] != ']')       // skip []
            j++;
    if (s1[i] != s2[j])
        return s1[i] - s2[j];
    }
return 0;
}

//-----------------------------------------------------------------------------
int main (int argc, char *argv[])
{
if (argc != 3)
    Err("mdiff version 1.2\nusage: mdiff file1 file2  (file2 can be a directory)");

FILE *pFile1 = fopen(argv[1], "r");
if (NULL == pFile1)
    Err("mdiff: can't open %s", argv[1]);

char sFile2[MAX_LEN]; strcpy(sFile2, argv[2]);
int nLen = strlen(sFile2);
if (sFile2[nLen-1] == '\\' || sFile2[nLen-1] == '/') // remove trailing slash if any
    sFile2[nLen-1] = 0;

// if argv[2] is a directory, create a path by prepending the dir to the basename of argv[1]
#if _MSC_VER // microsoft only to avoid hassles with _splitpath
struct _stat st;
if (_stat(sFile2, &st) == 0 && (st.st_mode & S_IFDIR))
    {
    // sFile2 is a directory
    char sBase1[1024], sExt1[1024];
    _splitpath(argv[1], NULL, NULL, sBase1, sExt1);
    _makepath(sFile2, NULL, argv[2], sBase1, sExt1);
    for (int i = 0; sFile2[i]; i++) // convert backslashes (created by makepath) to forward slashes
        if (sFile2[i] == '\\')
            sFile2[i] = '/';
    }
#else
struct stat st;
if (stat(sFile2, &st) == 0 && (st.st_mode & S_IFDIR))
    Err("%s is a directory, not supported in this version of mdiff", sFile2);
#endif
FILE *pFile2 = fopen(sFile2, "r");
if (NULL == pFile2)
    Err("mdiff: can't open %s", sFile2);

// following are static simply to keep these big variables off the stack
static char sLine1[MAX_LEN+1], sLine2[MAX_LEN+1];
static char LastLine1[MAX_LEN+1], LastLine2[MAX_LEN+1];
int iLine = 0, iErr = 0;

bool fLastLineHasErr = false;

while (fgets1(sLine1, MAX_LEN, pFile1))
    {
    // fLineHasErr prevents us printing the same line twice, if it has multiple errors
    bool fLineHasErr = false;
    iLine++;
    if (!fgets1(sLine2, MAX_LEN, pFile2))
        {
        if (iErr++ < MAX_DIFFS)                         // can't get line from file2
            PrintErr(iLine, argv[1], sLine1, sFile2, "SHORT FILE");
        iErr = MAX_DIFFS;                               // prevent further messages
        fLineHasErr = true;
        break;
        }
    if (fLastLineHasErr)
        {
        // basic resync (allows resynch after extra lines in one of the input files)

        if (0 == iStrCmpSkipBrackets(sLine1, LastLine2))
            fgets1(sLine1, MAX_LEN, pFile1);

        if (0 == iStrCmpSkipBrackets(sLine2, LastLine1))
            fgets1(sLine2, MAX_LEN, pFile2);
        }
    int i, j;
    for (i = 0, j = 0; sLine1[i] && sLine2[j]; i++, j++)
        {
        if (sLine1[i] == '[')                           // skip []
            while (sLine1[i] && sLine1[i] != ']')
                i++;
        if (sLine2[j] == '[')
            while (sLine2[j] && sLine2[j] != ']')       // skip []
                j++;
        if (sLine1[i] != sLine2[j])
            {
            if (!fLastLineHasErr            // don't print consecutive differences
                    && iErr++ < MAX_DIFFS)
                PrintErr(iLine, argv[1], sLine1, sFile2, sLine2);
            fLineHasErr = true;
            fLastLineHasErr = true;
            break;
            }
        }
    if (!fLineHasErr)
        fLastLineHasErr = false;
    if (!fLineHasErr && (sLine1[i] != 0 || sLine2[j] != 0)  // different line lengths?
            && iErr++ < MAX_DIFFS)
        {
        PrintErr(iLine, argv[1], sLine1, sFile2, sLine2);
        fLastLineHasErr = true;
        }
    if (iErr >= MAX_DIFFS)
        {
        printf("Reached MAX_DIFFS %d\n", MAX_DIFFS);
        break;
        }
    strcpy(LastLine1, sLine1);
    strcpy(LastLine2, sLine2);
    }
if (fgets1(sLine2, MAX_LEN, pFile2))     // extra line(s) in File2?
    {
    iLine++;
    if (iErr++ < MAX_DIFFS)
        PrintErr(iLine, argv[1], "SHORT FILE", sFile2, sLine2);
    }
return iErr != 0;   // return 0 on success
}
