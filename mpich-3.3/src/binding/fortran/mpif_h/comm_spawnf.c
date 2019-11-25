/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*  
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 * This file is automatically generated by buildiface 
 * DO NOT EDIT
 */
#include "mpi_fortimpl.h"


/* Begin MPI profiling block */
#if defined(USE_WEAK_SYMBOLS) && !defined(USE_ONLY_MPI_NAMES) 
#if defined(HAVE_MULTIPLE_PRAGMA_WEAK)
extern FORT_DLL_SPEC void FORT_CALL MPI_COMM_SPAWN( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn__( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn_( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );

#if defined(F77_NAME_UPPER)
#pragma weak MPI_COMM_SPAWN = PMPI_COMM_SPAWN
#pragma weak mpi_comm_spawn__ = PMPI_COMM_SPAWN
#pragma weak mpi_comm_spawn_ = PMPI_COMM_SPAWN
#pragma weak mpi_comm_spawn = PMPI_COMM_SPAWN
#elif defined(F77_NAME_LOWER_2USCORE)
#pragma weak MPI_COMM_SPAWN = pmpi_comm_spawn__
#pragma weak mpi_comm_spawn__ = pmpi_comm_spawn__
#pragma weak mpi_comm_spawn_ = pmpi_comm_spawn__
#pragma weak mpi_comm_spawn = pmpi_comm_spawn__
#elif defined(F77_NAME_LOWER_USCORE)
#pragma weak MPI_COMM_SPAWN = pmpi_comm_spawn_
#pragma weak mpi_comm_spawn__ = pmpi_comm_spawn_
#pragma weak mpi_comm_spawn_ = pmpi_comm_spawn_
#pragma weak mpi_comm_spawn = pmpi_comm_spawn_
#else
#pragma weak MPI_COMM_SPAWN = pmpi_comm_spawn
#pragma weak mpi_comm_spawn__ = pmpi_comm_spawn
#pragma weak mpi_comm_spawn_ = pmpi_comm_spawn
#pragma weak mpi_comm_spawn = pmpi_comm_spawn
#endif



#elif defined(HAVE_PRAGMA_WEAK)

#if defined(F77_NAME_UPPER)
extern FORT_DLL_SPEC void FORT_CALL MPI_COMM_SPAWN( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );

#pragma weak MPI_COMM_SPAWN = PMPI_COMM_SPAWN
#elif defined(F77_NAME_LOWER_2USCORE)
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn__( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );

#pragma weak mpi_comm_spawn__ = pmpi_comm_spawn__
#elif !defined(F77_NAME_LOWER_USCORE)
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );

#pragma weak mpi_comm_spawn = pmpi_comm_spawn
#else
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn_( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );

#pragma weak mpi_comm_spawn_ = pmpi_comm_spawn_
#endif

#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#if defined(F77_NAME_UPPER)
#pragma _HP_SECONDARY_DEF PMPI_COMM_SPAWN  MPI_COMM_SPAWN
#elif defined(F77_NAME_LOWER_2USCORE)
#pragma _HP_SECONDARY_DEF pmpi_comm_spawn__  mpi_comm_spawn__
#elif !defined(F77_NAME_LOWER_USCORE)
#pragma _HP_SECONDARY_DEF pmpi_comm_spawn  mpi_comm_spawn
#else
#pragma _HP_SECONDARY_DEF pmpi_comm_spawn_  mpi_comm_spawn_
#endif

#elif defined(HAVE_PRAGMA_CRI_DUP)
#if defined(F77_NAME_UPPER)
#pragma _CRI duplicate MPI_COMM_SPAWN as PMPI_COMM_SPAWN
#elif defined(F77_NAME_LOWER_2USCORE)
#pragma _CRI duplicate mpi_comm_spawn__ as pmpi_comm_spawn__
#elif !defined(F77_NAME_LOWER_USCORE)
#pragma _CRI duplicate mpi_comm_spawn as pmpi_comm_spawn
#else
#pragma _CRI duplicate mpi_comm_spawn_ as pmpi_comm_spawn_
#endif

#elif defined(HAVE_WEAK_ATTRIBUTE)
#if defined(F77_NAME_UPPER)
extern FORT_DLL_SPEC void FORT_CALL MPI_COMM_SPAWN( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("PMPI_COMM_SPAWN")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn__( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("PMPI_COMM_SPAWN")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn_( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("PMPI_COMM_SPAWN")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("PMPI_COMM_SPAWN")));

#elif defined(F77_NAME_LOWER_2USCORE)
extern FORT_DLL_SPEC void FORT_CALL MPI_COMM_SPAWN( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn__")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn__( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn__")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn_( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn__")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn__")));

#elif defined(F77_NAME_LOWER_USCORE)
extern FORT_DLL_SPEC void FORT_CALL MPI_COMM_SPAWN( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn_")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn__( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn_")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn_( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn_")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn_")));

#else
extern FORT_DLL_SPEC void FORT_CALL MPI_COMM_SPAWN( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn__( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn_( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn")));

#endif
#endif /* HAVE_PRAGMA_WEAK */
#endif /* USE_WEAK_SYMBOLS */
/* End MPI profiling block */


/* These definitions are used only for generating the Fortran wrappers */
#if defined(USE_WEAK_SYMBOLS) && defined(USE_ONLY_MPI_NAMES)
#if defined(HAVE_MULTIPLE_PRAGMA_WEAK)
extern FORT_DLL_SPEC void FORT_CALL MPI_COMM_SPAWN( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn__( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn_( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );

#if defined(F77_NAME_UPPER)
#pragma weak mpi_comm_spawn__ = MPI_COMM_SPAWN
#pragma weak mpi_comm_spawn_ = MPI_COMM_SPAWN
#pragma weak mpi_comm_spawn = MPI_COMM_SPAWN
#elif defined(F77_NAME_LOWER_2USCORE)
#pragma weak MPI_COMM_SPAWN = mpi_comm_spawn__
#pragma weak mpi_comm_spawn_ = mpi_comm_spawn__
#pragma weak mpi_comm_spawn = mpi_comm_spawn__
#elif defined(F77_NAME_LOWER_USCORE)
#pragma weak MPI_COMM_SPAWN = mpi_comm_spawn_
#pragma weak mpi_comm_spawn__ = mpi_comm_spawn_
#pragma weak mpi_comm_spawn = mpi_comm_spawn_
#else
#pragma weak MPI_COMM_SPAWN = mpi_comm_spawn
#pragma weak mpi_comm_spawn__ = mpi_comm_spawn
#pragma weak mpi_comm_spawn_ = mpi_comm_spawn
#endif
#elif defined(HAVE_WEAK_ATTRIBUTE)
#if defined(F77_NAME_UPPER)
extern FORT_DLL_SPEC void FORT_CALL MPI_COMM_SPAWN( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn__( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("MPI_COMM_SPAWN")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn_( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("MPI_COMM_SPAWN")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("MPI_COMM_SPAWN")));

#elif defined(F77_NAME_LOWER_2USCORE)
extern FORT_DLL_SPEC void FORT_CALL MPI_COMM_SPAWN( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("mpi_comm_spawn__")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn__( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn_( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("mpi_comm_spawn__")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("mpi_comm_spawn__")));

#elif defined(F77_NAME_LOWER_USCORE)
extern FORT_DLL_SPEC void FORT_CALL MPI_COMM_SPAWN( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("mpi_comm_spawn_")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn__( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("mpi_comm_spawn_")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn_( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("mpi_comm_spawn_")));

#else
extern FORT_DLL_SPEC void FORT_CALL MPI_COMM_SPAWN( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("mpi_comm_spawn")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn__( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("mpi_comm_spawn")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn_( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("mpi_comm_spawn")));
extern FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );

#endif
#endif

#endif

/* Map the name to the correct form */
#ifndef MPICH_MPI_FROM_PMPI
#if defined(USE_WEAK_SYMBOLS)
#if defined(HAVE_MULTIPLE_PRAGMA_WEAK)
/* Define the weak versions of the PMPI routine*/
#ifndef F77_NAME_UPPER
extern FORT_DLL_SPEC void FORT_CALL PMPI_COMM_SPAWN( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );
#endif
#ifndef F77_NAME_LOWER_2USCORE
extern FORT_DLL_SPEC void FORT_CALL pmpi_comm_spawn__( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );
#endif
#ifndef F77_NAME_LOWER_USCORE
extern FORT_DLL_SPEC void FORT_CALL pmpi_comm_spawn_( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );
#endif
#ifndef F77_NAME_LOWER
extern FORT_DLL_SPEC void FORT_CALL pmpi_comm_spawn( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL );

#endif

#if defined(F77_NAME_UPPER)
#pragma weak pmpi_comm_spawn__ = PMPI_COMM_SPAWN
#pragma weak pmpi_comm_spawn_ = PMPI_COMM_SPAWN
#pragma weak pmpi_comm_spawn = PMPI_COMM_SPAWN
#elif defined(F77_NAME_LOWER_2USCORE)
#pragma weak PMPI_COMM_SPAWN = pmpi_comm_spawn__
#pragma weak pmpi_comm_spawn_ = pmpi_comm_spawn__
#pragma weak pmpi_comm_spawn = pmpi_comm_spawn__
#elif defined(F77_NAME_LOWER_USCORE)
#pragma weak PMPI_COMM_SPAWN = pmpi_comm_spawn_
#pragma weak pmpi_comm_spawn__ = pmpi_comm_spawn_
#pragma weak pmpi_comm_spawn = pmpi_comm_spawn_
#else
#pragma weak PMPI_COMM_SPAWN = pmpi_comm_spawn
#pragma weak pmpi_comm_spawn__ = pmpi_comm_spawn
#pragma weak pmpi_comm_spawn_ = pmpi_comm_spawn
#endif /* Test on name mapping */

#elif defined(HAVE_WEAK_ATTRIBUTE)
#if defined(F77_NAME_UPPER)
extern FORT_DLL_SPEC void FORT_CALL pmpi_comm_spawn__( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("PMPI_COMM_SPAWN")));
extern FORT_DLL_SPEC void FORT_CALL pmpi_comm_spawn_( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("PMPI_COMM_SPAWN")));
extern FORT_DLL_SPEC void FORT_CALL pmpi_comm_spawn( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("PMPI_COMM_SPAWN")));

#elif defined(F77_NAME_LOWER_2USCORE)
extern FORT_DLL_SPEC void FORT_CALL PMPI_COMM_SPAWN( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn__")));
extern FORT_DLL_SPEC void FORT_CALL pmpi_comm_spawn_( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn__")));
extern FORT_DLL_SPEC void FORT_CALL pmpi_comm_spawn( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn__")));

#elif defined(F77_NAME_LOWER_USCORE)
extern FORT_DLL_SPEC void FORT_CALL PMPI_COMM_SPAWN( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn_")));
extern FORT_DLL_SPEC void FORT_CALL pmpi_comm_spawn__( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn_")));
extern FORT_DLL_SPEC void FORT_CALL pmpi_comm_spawn( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn_")));

#else
extern FORT_DLL_SPEC void FORT_CALL PMPI_COMM_SPAWN( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn")));
extern FORT_DLL_SPEC void FORT_CALL pmpi_comm_spawn__( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn")));
extern FORT_DLL_SPEC void FORT_CALL pmpi_comm_spawn_( char * FORT_MIXED_LEN_DECL, char * FORT_MIXED_LEN_DECL, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint *, MPI_Fint [], MPI_Fint * FORT_END_LEN_DECL FORT_END_LEN_DECL ) __attribute__((weak,alias("pmpi_comm_spawn")));

#endif /* Test on name mapping */
#endif /* HAVE_MULTIPLE_PRAGMA_WEAK */
#endif /* USE_WEAK_SYMBOLS */

#ifdef F77_NAME_UPPER
#define mpi_comm_spawn_ PMPI_COMM_SPAWN
#elif defined(F77_NAME_LOWER_2USCORE)
#define mpi_comm_spawn_ pmpi_comm_spawn__
#elif !defined(F77_NAME_LOWER_USCORE)
#define mpi_comm_spawn_ pmpi_comm_spawn
#else
#define mpi_comm_spawn_ pmpi_comm_spawn_
#endif /* Test on name mapping */

#ifdef F77_USE_PMPI
/* This defines the routine that we call, which must be the PMPI version
   since we're renaming the Fortran entry as the pmpi version.  The MPI name
   must be undefined first to prevent any conflicts with previous renamings. */
#undef MPI_Comm_spawn
#define MPI_Comm_spawn PMPI_Comm_spawn 
#endif

#else

#ifdef F77_NAME_UPPER
#define mpi_comm_spawn_ MPI_COMM_SPAWN
#elif defined(F77_NAME_LOWER_2USCORE)
#define mpi_comm_spawn_ mpi_comm_spawn__
#elif !defined(F77_NAME_LOWER_USCORE)
#define mpi_comm_spawn_ mpi_comm_spawn
/* Else leave name alone */
#endif


#endif /* MPICH_MPI_FROM_PMPI */

/* Prototypes for the Fortran interfaces */
#include "fproto.h"
FORT_DLL_SPEC void FORT_CALL mpi_comm_spawn_ ( char *v1 FORT_MIXED_LEN(d1), char *v2 FORT_MIXED_LEN(d2), MPI_Fint *v3, MPI_Fint *v4, MPI_Fint *v5, MPI_Fint *v6, MPI_Fint *v7, MPI_Fint v8[], MPI_Fint *ierr FORT_END_LEN(d1) FORT_END_LEN(d2) ){
    char *p1;
    char **p2;
    char *pcpy2;
    int  asize2=0;

    {char *p = v1 + d1 - 1;
     int  li;
        while (*p == ' ' && p > v1) p--;
        p++;
        p1 = (char *)MPL_malloc( p-v1 + 1, MPL_MEM_OTHER );
        for (li=0; li<(p-v1); li++) { p1[li] = v1[li]; }
        p1[li] = 0; 
    }

#ifndef HAVE_MPI_F_INIT_WORKS_WITH_C
    if (MPIR_F_NeedInit){ mpirinitf_(); MPIR_F_NeedInit = 0; }
#endif

    { int i;
      char *ptmp = NULL;

      /* Compute the size of the array by looking for an all-blank line */
      pcpy2 = v2;
      for (asize2=1; 1; asize2++) {
          char *pt = pcpy2 + d2 - 1;
          while (*pt == ' ' && pt > pcpy2) pt--;
          if (*pt == ' ') break;
          pcpy2 += d2;
      }

      p2 = (char **)MPL_malloc( asize2 * sizeof(char *), MPL_MEM_OTHER );
      if (asize2-1 > 0) ptmp = (char *)MPL_malloc( asize2 * (d2 + 1), MPL_MEM_OTHER );
      for (i=0; i<asize2-1; i++) {
          char *p = v2 + i * d2, *pin, *pdest;
          int j;

          pdest = ptmp + i * (d2 + 1);
          p2[i] = pdest;
          /* Move to the end and work back */
          pin = p + d2 - 1;
          while (*pin == ' ' && pin > p) pin--;
          /* Copy and then null terminate */
          for (j=0; j<(pin-p)+1; j++) { pdest[j] = p[j]; }
          pdest[j] = 0;
          }
    /* Null terminate the array */
    p2[asize2-1] = 0;
    }

    if ((MPI_Fint*)v8 == MPI_F_ERRCODES_IGNORE) { v8 = (MPI_Fint *)MPI_ERRCODES_IGNORE; }
    *ierr = MPI_Comm_spawn( p1, p2, (int)*v3, (MPI_Info)(*v4), (int)*v5, (MPI_Comm)(*v6), (MPI_Comm *)(v7), (int *)v8 );
    MPL_free( p1 );
    if (asize2-1 > 0) MPL_free( p2[0] );
    MPL_free( p2 );
}