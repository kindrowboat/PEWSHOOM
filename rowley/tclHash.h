/*
 * tcl.h --
 *
 *      This header file describes the externally-visible facilities
 *      of the Tcl interpreter.
 *
 * Copyright (c) 1987-1994 The Regents of the University of California.
 * Copyright (c) 1994-1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) tcl.h 1.174 95/09/12 12:42:42 modified for stasm
 */

#ifndef _TCL
#define _TCL

#ifndef BUFSIZ
#include <stdio.h>
#endif

#define TCL_VERSION "7.5 modified for stasm"
#define TCL_MAJOR_VERSION 7
#define TCL_MINOR_VERSION 5

/*
 * Definitions that allow Tcl functions with variable numbers of
 * arguments to be used with either varargs.h or stdarg.h.  TCL_VARARGS
 * is used in procedure prototypes.  TCL_VARARGS_DEF is used to declare
 * the arguments in a function definiton: it takes the type and name of
 * the first argument and supplies the appropriate argument declaration
 * string for use in the function definition.  TCL_VARARGS_START
 * initializes the va_list data structure and returns the first argument.
 */

#if defined(__STDC__) || defined(HAS_STDARG)
#   define TCL_VARARGS(type, name) (type name, ...)
#   define TCL_VARARGS_DEF(type, name) (type name, ...)
#   define TCL_VARARGS_START(type, name, list) (va_start(list, name), name)
#else
#   define TCL_VARARGS(type, name) ()
#   define TCL_VARARGS_DEF(type, name) (va_alist)
#   define TCL_VARARGS_START(type, name, list) \
        (va_start(list), va_arg(list, type))
#endif

/*
 * Definitions that allow this header file to be used either with or
 * without ANSI C features like function prototypes.
 */

#undef _ANSI_ARGS_
#undef CONST

#if ((defined(__STDC__) || defined(SABER)) && !defined(NO_PROTOTYPE)) || defined(__cplusplus) || defined(USE_PROTOTYPE)
#   define _USING_PROTOTYPES_ 1
#   define _ANSI_ARGS_(x)       x
#   define CONST const
#else
#   define _ANSI_ARGS_(x)       ()
#   define CONST
#endif

#ifdef __cplusplus
#   define EXTERN_TCLHASH extern "C"
#else
#   define EXTERN_TCLHASH extern
#endif

/*
 * Macro to use instead of "void" for arguments that must have
 * type "void *" in ANSI C;  maps them to type "char *" in
 * non-ANSI systems.
 */
#ifndef _Windows
#ifndef VOID
#   ifdef __STDC__
#       define VOID void
#   else
#       define VOID char
#   endif
#endif
#else /* !_Windows */
/*
 * The following code is copied from winnt.h
 */
#ifndef VOID
#define VOID void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
#endif
#endif /* !_Windows */

/*
 * Miscellaneous declarations (to allow Tcl to be used stand-alone,
 * without the rest of Sprite).
 */

#ifndef NULL
#define NULL 0
#endif

#ifndef _CLIENTDATA
#   if defined(__STDC__) || defined(__cplusplus)
    typedef void *ClientData;
#   else
    typedef int *ClientData;
#   endif /* __STDC__ */
#define _CLIENTDATA
#endif

/*
 * Forward declaration of Tcl_HashTable.  Needed by some C++ compilers
 * to prevent errors when the forward reference to Tcl_HashTable is
 * encountered in the Tcl_HashEntry structure.
 */

#ifdef __cplusplus
struct Tcl_HashTable;
#endif

/*
 * Structure definition for an entry in a hash table.  No-one outside
 * Tcl should access any of these fields directly;  use the macros
 * defined below.
 */

typedef struct Tcl_HashEntry {
    struct Tcl_HashEntry *nextPtr;      /* Pointer to next entry in this
                                         * hash bucket, or NULL for end of
                                         * chain. */
    struct Tcl_HashTable *tablePtr;     /* Pointer to table containing entry. */
    struct Tcl_HashEntry **bucketPtr;   /* Pointer to bucket that points to
                                         * first entry in this entry's chain:
                                         * used for deleting the entry. */
    ClientData clientData;              /* Application stores something here
                                         * with Tcl_SetHashValue. */
    union {                             /* Key has one of these forms: */
        char *oneWordValue;             /* One-word value for key. */
        int words[1];                   /* Multiple integer words for key.
                                         * The actual size will be as large
                                         * as necessary for this table's
                                         * keys. */
        char string[4];                 /* String for key.  The actual size
                                         * will be as large as needed to hold
                                         * the key. */
    } key;                              /* MUST BE LAST FIELD IN RECORD!! */
} Tcl_HashEntry;

/*
 * Structure definition for a hash table.  Must be in tcl.h so clients
 * can allocate space for these structures, but clients should never
 * access any fields in this structure.
 */

#define TCL_SMALL_HASH_TABLE 4
typedef struct Tcl_HashTable {
    Tcl_HashEntry **buckets;            /* Pointer to bucket array.  Each
                                         * element points to first entry in
                                         * bucket's hash chain, or NULL. */
    Tcl_HashEntry *staticBuckets[TCL_SMALL_HASH_TABLE];
                                        /* Bucket array used for small tables
                                         * (to avoid mallocs and frees). */
    int numBuckets;                     /* Total number of buckets allocated
                                         * at **bucketPtr. */
    int numEntries;                     /* Total number of entries present
                                         * in table. */
    int rebuildSize;                    /* Enlarge table when numEntries gets
                                         * to be this large. */
    int downShift;                      /* Shift count used in hashing
                                         * function.  Designed to use high-
                                         * order bits of randomized keys. */
    int mask;                           /* Mask value used in hashing
                                         * function. */
    int keyType;                        /* Type of keys used in this table.
                                         * It's either TCL_STRING_KEYS,
                                         * TCL_ONE_WORD_KEYS, or an integer
                                         * giving the number of ints in a
                                         */
    Tcl_HashEntry *(*findProc) _ANSI_ARGS_((struct Tcl_HashTable *tablePtr,
            char *key));
    Tcl_HashEntry *(*createProc) _ANSI_ARGS_((struct Tcl_HashTable *tablePtr,
            char *key, int *newPtr));
} Tcl_HashTable;

/*
 * Structure definition for information used to keep track of searches
 * through hash tables:
 */

typedef struct Tcl_HashSearch {
    Tcl_HashTable *tablePtr;            /* Table being searched. */
    int nextIndex;                      /* Index of next bucket to be
                                         * enumerated after present one. */
    Tcl_HashEntry *nextEntryPtr;        /* Next entry to be enumerated in the
                                         * the current bucket. */
} Tcl_HashSearch;

/*
 * Acceptable key types for hash tables:
 */

#define TCL_STRING_KEYS         0
#define TCL_ONE_WORD_KEYS       1

/*
 * Macros for clients to use to access fields of hash entries:
 */

#define Tcl_GetHashValue(h) ((h)->clientData)
#define Tcl_SetHashValue(h, value) ((h)->clientData = (ClientData) (value))
#define Tcl_GetHashKey(tablePtr, h) \
    ((char *) (((tablePtr)->keyType == TCL_ONE_WORD_KEYS) ? (h)->key.oneWordValue \
                                                : (h)->key.string))

/*
 * Macros to use for clients to use to invoke find and create procedures
 * for hash tables:
 */

#define Tcl_FindHashEntry(tablePtr, key) \
        (*((tablePtr)->findProc))(tablePtr, key)
#define Tcl_CreateHashEntry(tablePtr, key, newPtr) \
        (*((tablePtr)->createProc))(tablePtr, key, newPtr)

EXTERN_TCLHASH void             Tcl_DeleteHashEntry _ANSI_ARGS_((
                            Tcl_HashEntry *entryPtr));

EXTERN_TCLHASH void             Tcl_DeleteHashTable _ANSI_ARGS_((
                            Tcl_HashTable *tablePtr));

EXTERN_TCLHASH Tcl_HashEntry *  Tcl_FirstHashEntry _ANSI_ARGS_((
                            Tcl_HashTable *tablePtr,
                            Tcl_HashSearch *searchPtr));

EXTERN_TCLHASH char *           Tcl_HashStats _ANSI_ARGS_((Tcl_HashTable *tablePtr));

EXTERN_TCLHASH void             Tcl_InitHashTable _ANSI_ARGS_((Tcl_HashTable *tablePtr,
                            int keyType));

EXTERN_TCLHASH Tcl_HashEntry *  Tcl_NextHashEntry _ANSI_ARGS_((
                            Tcl_HashSearch *searchPtr));

#endif /* _TCL */
