/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/sys/coroutine.h"
#include "common/sys/threads.h"
#include <sys/time.h>

void dice(void* p)
{
    void* p1 = cxx::sys::taskarg::p1(p);
    void* p2 = cxx::sys::taskarg::p2(p);

    cxx::sys::coroutine::name(p1, "dice %d", cxx::sys::coroutine::id(p1));

    timeval tv;
    gettimeofday(&tv, NULL);
    srandom(tv.tv_usec);
    int sum = 0;
    while((sum += random() % 6 + 1) < 10000) {
        cxx::sys::coroutine::yield(p1);
    }

    cxx::sys::coroutine::check(p1);
    printf("winner is %s %d, %d\n", cxx::sys::coroutine::name(p1), cxx::sys::coroutine::id(p1), sum);
    cxx::sys::coroutine::quit(p1, 0);
}

void dice_main(int argc, char** argv)
{
    cxx::sys::coroutine* c = (cxx::sys::coroutine* )argv[1];
    for(long i = 0; i < 5; ++i) {
        c->create(dice, (void* )i, 4096);
    }
}

TEST(coroutine, dice)
{
    cxx::sys::coroutine c(256*1024);

    char** argv = new char*[2];
    argv[0] = new char[5];
    argv[1] = (char* )&c;
    strcpy(argv[0], "dice");

    c.start(dice_main, 2, argv);

    printf("dice done\n");
}

void perf(void* p)
{
    void* p1 = cxx::sys::taskarg::p1(p);
    void* p2 = cxx::sys::taskarg::p2(p);

    int count = 0;
    while(count++ < 1000000) {
        cxx::sys::coroutine::yield(p1);
    }
}

void perf_main(int argc, char** argv)
{
    cxx::sys::coroutine* c = (cxx::sys::coroutine* )argv[1];
    for(long i = 0; i < 5; ++i) {
        c->create(perf, (void* )i, 4096);
    }
}

TEST(coroutine, perfmance)
{
    // limit the thread to a CPU
    int cpu[5] = { 0, 1, 2, 3, 4 };
    int ret = cxx::sys::threadcontrol::policy(1, cpu + 1);

    cxx::sys::coroutine c(256*1024);

    char** argv = new char*[2];
    argv[0] = new char[5];
    argv[1] = (char* )&c;
    strcpy(argv[0], "perf");

    c.start(perf_main, 2, argv);

    printf("perf done\n");
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
