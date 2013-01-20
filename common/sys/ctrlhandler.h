/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef	CXX_SYS_CTRLHANDLER_H
#define	CXX_SYS_CTRLHANDLER_H

namespace cxx {
    namespace sys {

        typedef void (*CtrlHandlerCallBack)(int);
        class ctrlhandler {
        public:
            explicit ctrlhandler(CtrlHandlerCallBack callback= 0);
            ~ctrlhandler();

            void sethandler(CtrlHandlerCallBack callback);
            CtrlHandlerCallBack gethandler() const;
        };

    }
}

#endif
