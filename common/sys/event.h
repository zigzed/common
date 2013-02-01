/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_SYS_EVENT_H
#define CXX_SYS_EVENT_H
#include "common/config.h"
#if defined(OS_WINDOWS)
    #include <winsock2.h>
#endif

namespace cxx {
    namespace sys {

        /** this is a cross platform event object that can be used as an event
         * wait/notify mechanism by user-space applications, and by the kernel
         * to notify user-space applications of events.
         *
         * event::handle() can be used by select/poll/epoll under linux, and
         * used by select under windows.
         */
        class event {
        public:
#if     defined(OS_LINUX)
            typedef int     handle_t;
#elif   defined(OS_WINDOWS)
            typedef SOCKET  handle_t;
#endif
            event();
            ~event();

            handle_t    handle() const;
            void        send();
            bool        wait(int timeout);
            void        recv();
        private:
            event(const event& rhs);
            event& operator= (const event& rhs);

            handle_t    w_;
            handle_t    r_;
        };

    }
}

#endif
