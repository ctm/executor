// api-module.h

// set up TRAP() macros and associated template instantiation for one header file
// included multiple times on purpose, definitely no #pragma once or #include guards

#ifndef MODULE_NAME
#error "MODULE_NAME must be defined before including rsys/api-module.h"
#endif

#define PREPROCESSOR_CONCAT1D(A,B) defined(A##B)
#define PREPROCESSOR_CONCATD(A,B) PREPROCESSOR_CONCAT1D(A,B)

#undef TRAP_INSTANTIATION

#if PREPROCESSOR_CONCATD(INSTANTIATE_TRAPS_, MODULE_NAME)
#define TRAP_INSTANTIATION DEFINE

#include <rsys/traps.impl.h>
#else
#define TRAP_INSTANTIATION EXTERN
#endif

#undef MODULE_NAME
