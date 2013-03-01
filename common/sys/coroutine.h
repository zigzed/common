/** Copyright (C) wilburlang@gmail.com
 * based on Russ Cox's libtask
 */
#ifndef CXX_SYS_COROUTINE_H
#define CXX_SYS_COROUTINE_H
#include <stddef.h>

namespace cxx {
    namespace sys {

        typedef void (*taskptr)(void*);

        class taskarg {
        public:
            static void* p1(void* );
            static void* p2(void* );
        };

        class coroutine {
        public:
            struct task;
            struct context;

            explicit coroutine(int stack_size);
            ~coroutine();

            /** create a coroutine task from the given function */
            task*   create(taskptr func, void* arg, int stack);

            /** starting the coroutine with entry function fn */
            int     start(void (*fn)(int, char** ), int argc, char** argv);

            /** give up the CPU and switch to other tasks
             * return number of other tasks will be scheduled run
             */
            static int          yield(void* task);

            /** give up the CPU and switch to other tasks at least ms milliseconds */
            static int          delay(void* task, int ms);

            /** return a pointer to a per-task void* pointer */
            static void**       data(void* task);

            /** set name of the task */
            static void         name(void* task, char* fmt, ...);

            /** set state of the task */
            static void         info(void* task, char *fmt, ...);

            /** get name of the task */
            static const char*  name(void* task);

            /** get info of the task */
            static const char*  info(void* task);

            /** get id of the task */
            static unsigned int id(void* task);

            /** stop the task */
            static void         stop(void* task, int status);

            /** quit the coroutine */
            static void         quit(void* task, int status);

        private:
            int     schedule();
            void    add_task(task* t);
            void    del_task(task* t);
            void    taskready(task* t);
            void    taskshift();
            void    ctxtshift(task* v, context* f, context* t);
            void    needstack(task* t, int n);
            task*   taskalloc(taskptr p, void* arg, unsigned int stack);

            struct task_list {
                task* head;
                task* tail;
            };

            int         taskcounts_;
            int         taskswitch_;
            int         taskexit_;
            task*       running_;
            context*    pending_;
            task_list   tasklist_;
            task**      alltasks_;
            int         nalltask_;
            char*       argv0_;
            static int  idgen_;
            int         stack_;
        };
    }
}

#endif
