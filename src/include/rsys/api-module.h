// api-module.h

// set up TRAP() macros and associated template instantiation for one header file
// included multiple times on purpose, definitely no #pragma once or #include guards

#ifndef MODULE_NAME
#error "MODULE_NAME must be defined before including rsys/api-module.h"
#endif

#define PREPROCESSOR_CONCAT1(A,B) A##B
#define PREPROCESSOR_CONCAT(A,B) PREPROCESSOR_CONCAT1(A,B)

#undef TRAP_INSTANTIATION

#if PREPROCESSOR_CONCAT(INSTANTIATE_TRAP_, MODULE_NAME)
#define TRAP_INSTATIATION DEFINE
#else
#define TRAP_INSTATIATION EXTERN
#endif

#undef MODULE_NAME
