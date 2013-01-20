/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_LOG_LOG_H
#define CXX_LOG_LOG_H

namespace cxx {
    namespace log {

        class LogChannel;
        class LogPublisher;
        class LogNode;

        enum LogLevel {
            emerg   = 0,        ///< system is unusable
            alert   = 1,        ///< action must be taken immediately
            crit    = 2,        ///< critical conditions
            error   = 3,        ///< error conditions
            warning = 4,        ///< warning conditions
            notice  = 5,        ///< normal but significant condition
            info    = 6,        ///< informational
            debug   = 7,        ///< debug-level messages
            all     = 8,        ///< all level messages above
            undef   = 15
        };

        /** initialize the log modules with configure file*/
        void LogInit(const char* file, const char* module);

        /** pre-defined channels. "Debug", "Info", "Notice", "Warning",
         * "Error", "Critical", "Alert" and "Emergency"
         * And some rules for use different level:
         *
         * Emergency is used for (attempting to) record contract violations,
         * i.e. the firing of an active runtime contract, indicating that the
         * program is now behaving in contradiction to its design. According
         * to the principle of irrecoverability, a program in this state must
         * immediately terminate, and thus it is possible to see an emergency
         * level message being the last, or one of the last, messages in the
         * diagnostic logging sequence of a program that has faulted.
         *
         * Alert is used for (attempting to) record practically-unrecoverable
         * conditions, e.g. out of memory. It is customary to see an alert
         * level message being the last, or one of the last, messages in the
         * diagnostic logging sequence of a program that has ended processing
         * in a non-normative manner.
         *
         * Critical and error are used for conditions that usually indicate
         * that the normative behaviour of the program cannot be achieved.
         * There is some ambiguity as to the exact distinction to draw between
         * them, and I'm still in two minds about it. Obviously, critical is
         * more severe, and may be associated with conditions more likely to
         * result in program failure than does those designated as error.
         *
         * Warning is used for recording warning conditions.
         *
         * Notice is used for logging information that is to be displayed in
         * "normal" operation, e.g. "database connection achieved"
         *
         * Informational is used for logging information that is useful when
         * actively monitoring the health of a system, but that is not
         * necessarily displayed as part of "normal" operation. Ideally it
         * should be possible to turn on all logging of this level without
         * exceeding the program's performance criteria.
         *
         * Debug is used for debugging statements. Note that one of the
         * important advanges of using Pantheios is that its high efficiency
         * means that you don't have to make decisions about eliding certain
         * statements at compile-time; they can instead be made at runtime,
         * which is appropriate. Using Pantheios means eliminating #ifdef DEBUG
         * from application code, forever!
         */        

    }
}

#include "common/log/logmacro.h"

#endif
