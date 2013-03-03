/** Copyright (C) wilburlang@gmail.com
 * based on Russ Cox's libtask
 */
#ifndef CXX_CON_COROUTINE_H
#define CXX_CON_COROUTINE_H
#include <stddef.h>
#include <map>

namespace cxx {
    namespace con {

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
            task*   getcur();

            /** starting the coroutine with entry function fn */
            int     start(void (*fn)(int, char** ), int argc, char** argv);

            static task*        create(void* task, taskptr func, void *arg, int stack);

            /** give up the CPU and switch to other tasks
             * return number of other tasks will be scheduled run
             */
            static int          yield(void* task);

            /** give up the CPU and switch to other tasks at least ms milliseconds */
            static int          delay(void* task, int ms);

            /** return a pointer to a per-task void* pointer */
            static void**       data(void* task);

            /** set name of the task */
            static void         name(void* task, const char* fmt, ...);

            /** set state of the task */
            static void         state(void* task, const char *fmt, ...);

            /** get name of the task */
            static const char*  name(void* task);

            /** get info of the task */
            static const char*  state(void* task);

            /** get id of the task */
            static unsigned int id(void* task);

            /** stop the task */
            static void         stop(void* task, int status);

            /** quit the coroutine */
            static void         quit(void* task, int status);

            /** wait for object 'object' */
            static void         wait(void* task, int object);
            /** wakeup the task that wait for 'object' */
            static int          post(void* task, int object, int all);

            static int          check(void* task);
            static void         system(void* task);
        private:
            struct task_list {
                task* head;
                task* tail;
            };

            static void add_task(task_list& list, task* t);
            static void del_task(task_list& list, task* t);
            static void sleeptsk(void* arg);

            int     schedule();
            void    taskready(task* t);
            void    taskshift();
            void    ctxtshift(task* v, context* f, context* t);
            void    needstack(task* t, int n);
            task*   taskalloc(taskptr p, void* arg, unsigned int stack);

            int         taskcounts_;
            int         taskswitch_;
            int         taskexit_;
            task*       running_;
            context*    pending_;
            // active (running) tasks
            task_list   actives_;
            // sleeping tasks
            task_list   sleeping_;
            task**      alltasks_;
            int         nalltask_;
            static int  idgen_;
            int         stack_;
            int         sleepcnt_;

            typedef std::map<int, task_list >   wait_t;
            wait_t      waiting_;
        };
    }
}

#endif
