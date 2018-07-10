//change to INT or DOUBLE or pass via compiler flag
//#define DOUBLE
#ifndef INTRODUCTION_PARALLEL_COMPUTING_TYPES_H
#define INTRODUCTION_PARALLEL_COMPUTING_TYPES_H

#ifdef INT
#define FS "%i"
typedef int basetype;
#define TYPE_MPI MPI_INT
#endif
#ifdef DOUBLE
#define FS "%f"
typedef double basetype;
#define TYPE_MPI MPI_DOUBLE
#endif

#endif
