/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_COMMON_H
#define CXX_COMMON_H

#if !defined(OS_WINDOWS)
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN32__) || defined(WIN64)
        #define OS_WINDOWS
        #define WIN32_LEAN_AND_MEAN
        #include <windows.h>
    #endif
#endif

#if !defined(OS_MACOSX)
    #if defined(MAC_OSX)
        #define OS_MACOSX
    #endif
#endif

#if (!defined(OS_WINDOWS) && !defined(OS_MACOSX)) || defined(OS_LINUX)
    #define OS_LINUX
#endif

#if defined(OS_WINDOWS) && (defined(_MSC_VER) || defined(__BORLANDC__))
    typedef __int16             int16_t;
    typedef unsigned __int16    uint16_t;
    typedef __int32             int32_t;
    typedef unsigned __int32    uint32_t;
    typedef __int64             int64_t;
    typedef unsigned __int16    uint64_t;
#else
    #include <stdint.h>
#endif

#if defined(OS_WINDOWS) && (defined(_MSC_VER) || defined(__MINGW32__))
    #if defined(DLL_EXPORT)
        #define COMMON_API  __declspec(dllexport)
    #else
        #define COMMON_API  __declspec(dllimport)
    #endif
#else
    #define COMMON_API
#endif

#endif
