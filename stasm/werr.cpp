// $werr.cpp 3.0 milbo$ error reporting routines for a windowed environment
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

#ifdef WIN32
#include <windows.h>
#endif
#include "stasm.hpp"

bool fgErr;                         // set if there is an error
static char sgErr[MAX_PRINT_LEN];   // general purpose error msg buffer

//-----------------------------------------------------------------------------
// Print an error message and exit.
// This puts a \n on the msg so you shouldn't.

void Err (const char *pArgs, ...)       // args like printf
{
fgErr = true;   // set global error flag to minimize run-on errors

va_list pArg;
va_start(pArg, pArgs);
vsprintf(sgErr, pArgs, pArg);
va_end(pArg);

printf("\nError: %s\n", sgErr);
fflush(stdout);
if (pgLogFile)
    {
    fprintf(pgLogFile, "\nError: %s\n", sgErr);
    fflush(pgLogFile);
    }
extern const char *sgProgramName;
char sHeader[SLEN];
sprintf(sHeader, "%s error", sgProgramName);
sgErr[0] = toupper(sgErr[0]);
MessageBox(NULL, sgErr, sHeader, MB_OK);

#if _DEBUG
if (ENTER_DEBUGGER_ON_ERR)
    ENTER_DEBUGGER("ENTER_DEBUGGER_ON_ERR is true "
                   "so forcing entry to debugger");
#endif
}

//-----------------------------------------------------------------------------
void Warn (const char *pArgs, ...)  // args like printf
{
va_list pArg;
va_start(pArg, pArgs);
vsprintf(sgErr, pArgs, pArg);
va_end(pArg);
lprintf("Warning: %s\n", sgErr);
}

//-----------------------------------------------------------------------------
// same as Warn() but prefix a new line

void WarnWithNewLine (const char *pArgs, ...)   // args like printf
{
lprintf("\n");
va_list pArg;
va_start(pArg, pArgs);
vsprintf(sgErr, pArgs, pArg);
va_end(pArg);
lprintf("Warning: %s\n", sgErr);
}
