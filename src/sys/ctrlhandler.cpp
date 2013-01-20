/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include <cassert>
#include "common/config.h"
#include "common/sys/ctrlhandler.h"
#if defined(__sun)
#define _POSIX_PTHREAD_SEMANTICS
#endif
#if defined(OS_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#   include <signal.h>
#	include <pthread.h>
#endif

namespace cxx {
    namespace sys {

        static CtrlHandlerCallBack	theCallBack	= 0;
        static const ctrlhandler*	theHandlers	= 0;

        void ctrlhandler::sethandler(CtrlHandlerCallBack callback)
        {
            theCallBack = callback;
        }

        CtrlHandlerCallBack ctrlhandler::gethandler() const
        {
            return theCallBack;
        }

#if defined(OS_WINDOWS)

        static BOOL WINAPI handlerRoutine(DWORD dwCtrlType)
        {
            CtrlHandlerCallBack callback = theHandlers->gethandler();
            if(callback != NULL) {
                callback(dwCtrlType);
            }
            return TRUE;
        }

        ctrlhandler::ctrlhandler(CtrlHandlerCallBack callback)
        {
            if(callback == NULL) {
                assert(false);
            }
            else {
                theCallBack = callback;
                theHandlers = this;
                SetConsoleCtrlHandler(handlerRoutine, TRUE);
            }
        }

        ctrlhandler::~ctrlhandler()
        {
            SetConsoleCtrlHandler(handlerRoutine, FALSE);
            theHandlers = 0;
        }

#else
        extern "C" {
            static void* sigwaitThread(void*)
            {
                sigset_t ctrlCLikeSignals;
                sigemptyset(&ctrlCLikeSignals);
                sigaddset(&ctrlCLikeSignals, SIGHUP);
                sigaddset(&ctrlCLikeSignals, SIGINT);
                sigaddset(&ctrlCLikeSignals, SIGTERM);

                // Run until I'm cancelled (in sigwait())
                //
                for(;;) {
                    int signal = 0;
                    int rc = sigwait(&ctrlCLikeSignals, &signal);
                    assert(rc == 0);

                    rc = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);
                    assert(rc == 0);

                    CtrlHandlerCallBack callback = theHandlers->gethandler();

                    if(callback != 0) {
                        callback(signal);
                    }

                    rc = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
                    assert(rc == 0);
                }
                return 0;
            }
        }

        static pthread_t _tid;
        ctrlhandler::ctrlhandler(CtrlHandlerCallBack callback)
        {
            if(theHandlers != 0) {
                assert(false);
            }
            else {
                theCallBack = callback;
                theHandlers = this;

                // We block these CTRL+C like signals in the main thread,
                // and by default all other threads will inherit this signal
                // disposition.
                sigset_t ctrlCLikeSignals;
                sigemptyset(&ctrlCLikeSignals);
                sigaddset(&ctrlCLikeSignals, SIGHUP);
                sigaddset(&ctrlCLikeSignals, SIGINT);
                sigaddset(&ctrlCLikeSignals, SIGTERM);
                int rc = pthread_sigmask(SIG_BLOCK, &ctrlCLikeSignals, 0);
                assert(rc == 0);

                // Joinable thread
                rc = pthread_create(&_tid, 0, sigwaitThread, 0);
                assert(rc == 0);
            }
        }

        ctrlhandler::~ctrlhandler()
        {
            int rc = pthread_cancel(_tid);
            assert(rc == 0);
            void* status = 0;
            rc = pthread_join(_tid, &status);
            assert(rc == 0);
            assert(status == PTHREAD_CANCELED);
            theHandlers = 0;
        }
#endif
    }
}
