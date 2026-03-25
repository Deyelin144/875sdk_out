#ifndef _GULITESF_TYPE_H_
#define _GULITESF_TYPE_H_

#include "gulitesf_config.h"
#include <stdio.h>
#include <stdint.h>

// Define the PTR_TO_NUM macro for converting a pointer to a number
#if defined(_WIN32) || defined(_WIN64)
// Windows
#if defined(_WIN64)
    // 64-bit Windows
    #define PTR_TO_NUM(ptr) ((uint64_t)(uintptr_t)(ptr))
    #define PTR_FMT "llu"
    typedef uint64_t gulite_unumber_t;
    #define STRTONUM strtoull
#else
    // 32-bit Windows
    #define PTR_TO_NUM(ptr) ((uint32_t)(uintptr_t)(ptr))
    #define PTR_FMT "u"
    typedef uint32_t gulite_unumber_t;
    #define STRTONUM strtoul
#endif
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
// Unix-like systems (Linux, macOS, etc.)
#if defined(__x86_64__) || defined(__aarch64__) || (defined(__riscv) && __riscv_xlen == 64)
    // 64-bit Unix-like systems
    #define PTR_TO_NUM(ptr) ((uint64_t)(uintptr_t)(ptr))
    #define PTR_FMT "lu"
    typedef uint64_t gulite_unumber_t;
    #define STRTONUM strtoull
#else
    // 32-bit Unix-like systems
    #define PTR_TO_NUM(ptr) ((uint32_t)(uintptr_t)(ptr))
    #define PTR_FMT "u"
    typedef uint32_t gulite_unumber_t;
    #define STRTONUM strtoul
#endif
#else
// Fallback for other platforms
#if defined(__x86_64__) || defined(__aarch64__) || (defined(__riscv) && __riscv_xlen == 64)
    // 64-bit systems
    #define PTR_TO_NUM(ptr) ((uint64_t)(uintptr_t)(ptr))
    #define PTR_FMT "lu"
    typedef uint64_t gulite_unumber_t;
    #define STRTONUM strtoull
#else
    // 32-bit systems
    #define PTR_TO_NUM(ptr) ((uint32_t)(uintptr_t)(ptr))
    #define PTR_FMT "u"
    typedef uint32_t gulite_unumber_t;
    #define STRTONUM strtoul
#endif
#endif

#define NUM_TO_PTR(num) ((void *)(uintptr_t)(num))

#endif