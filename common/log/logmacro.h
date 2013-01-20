#ifndef CXX_LOG_LOGMACRO_H
#define CXX_LOG_LOGMACRO_H
#include "common/config.h"
#include "common/log/log.h"

namespace cxx {
    namespace log {

#define CONCAT2(A,B)            A##B
#define CONCAT(A,B)             CONCAT2(A,B)
#define STR(X)                  #X

#if defined  __GNUC__
#   define  PRINTF(FMT, X)      __attribute__ (( __format__ ( __printf__, FMT, X )))
#else
#   define  PRINTF(FMT, X)
#endif

#if __GNUC__ >= 3
#   define  expect(foo, bar)    __builtin_expect((foo), bar)
#   define  likely(x)           expect((x), 1)
#   define  unlikely(x)         expect((x), 0)
    void __checkArgs(int, const char*, ...) PRINTF(2, 3);
#else
#   define  expect(foo, bar)    (foo)
#   define  likely(x)           (x)
#   define  unlikely(x)         (x)
    inline void __checkArgs(int, const char*, ...) {}
#endif        

#define CONCAT2(A,B)            A##B
#define CONCAT(A,B)             CONCAT2(A,B)
#define STR(X)                  #X



#ifdef OS_WINDOWS
#   define  SUPPORT_VARIADICS_MACRO 1
#else
	#if __GNUC__ >= 3 || _MSC_VER_ >= 1400
	#   define  SUPPORT_VARIADICS_MACRO 1
	#else
	#   define  SUPPORT_VARIADICS_MACRO 0
	#endif
#endif

#if  SUPPORT_VARIADICS_MACRO

    void xMessage(const char* module, LogLevel faci, const char* file, int line, const char* func, const char* format, ...) PRINTF(6, 7);

#   define  xLog(MODULE, faci, format, ...) \
    do { \
        cxx::xlog::xMessage(MODULE, faci, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__); \
    } while(0)

#   define  xLog_IF(COND, MODULE, faci, format, ...) \
    do { \
        if(likely(COND)) {  \
            cxx::xlog::xMessage(MODULE, faci, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__); \
        } \
    } while(0)


#   define  xAssert(COND, MODULE, format, ...) \
    do { \
        if(unlikely(COND)) { \
            cxx::xlog::xMessage(MODULE, cxx::xlog::error, __FILE__, __LINE__, __FUNCTION__, "assert failed: %s", STR(COND)); \
            cxx::xlog::xMessage(MODULE, cxx::xlog::error, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__); \
        } \
    } while(0)

#   define  xDebug(MODULE, format, ... ) \
    do { \
        cxx::xlog::xMessage(MODULE, cxx::xlog::debug, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__); \
    } while(0)

#   define  xInfo(MODULE, format, ... ) \
    do { \
        cxx::xlog::xMessage(MODULE, cxx::xlog::info, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__); \
    } while(0)


#   define  xNotice(MODULE, format, ... ) \
    do { \
        cxx::xlog::xMessage(MODULE, cxx::xlog::notice, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__); \
    } while(0)


#   define  xWarning(MODULE, format, ... ) \
    do { \
        cxx::xlog::xMessage(MODULE, cxx::xlog::warning, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__); \
    } while(0)


#   define  xError(MODULE, format, ... ) \
    do { \
        cxx::xlog::xMessage(MODULE, cxx::xlog::error, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__); \
    } while(0)


#   define  xAlert(MODULE, format, ... ) \
    do { \
        cxx::xlog::xMessage(MODULE, cxx::xlog::alert, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__); \
    } while(0)

#   define  xCrit(MODULE, format, ... ) \
    do { \
        cxx::xlog::xMessage(MODULE, cxx::xlog::crit, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__); \
    } while(0)

#else
#   error "unknown compiler for supporting variadic macro"
#endif

#ifndef NDEBUG

#   define  dLog(MODULE, faci, format, ...) \
    do { \
        cxx::xlog::xMessage(MODULE, faci, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__); \
    } while(0)

#else

#   define  dLog(MODULE, faci, format, ...) \
    do { \
        ; \
    } while(0)

#endif

    }
}

#endif

