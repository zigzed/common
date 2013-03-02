/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/con/coroutine.h"
#include "common/sys/threads.h"
#include <sys/time.h>

void dice(void* p)
{
    void* p1 = cxx::con::taskarg::p1(p);
    void* p2 = cxx::con::taskarg::p2(p);

    cxx::con::coroutine::name(p1, "dice %d", cxx::con::coroutine::id(p1));

    timeval tv;
    gettimeofday(&tv, NULL);
    srandom(tv.tv_usec);
    int sum = 0;
    while((sum += random() % 6 + 1) < 10000) {
        cxx::con::coroutine::yield(p1);
    }

    cxx::con::coroutine::check(p1);
    printf("winner is %s %d, %d\n", cxx::con::coroutine::name(p1), cxx::con::coroutine::id(p1), sum);
    cxx::con::coroutine::quit(p1, 0);
}

void dice_main(int argc, char** argv)
{
    cxx::con::coroutine* c = (cxx::con::coroutine* )argv[1];
    for(long i = 0; i < 5; ++i) {
        c->create(dice, (void* )i, 4096);
    }
}

TEST(coroutine, dice)
{
    cxx::con::coroutine c(256*1024);

    char** argv = new char*[2];
    argv[0] = new char[5];
    argv[1] = (char* )&c;
    strcpy(argv[0], "dice");

    c.start(dice_main, 2, argv);

    printf("dice done\n");
}

void perf(void* p)
{
    void* p1 = cxx::con::taskarg::p1(p);
    void* p2 = cxx::con::taskarg::p2(p);

    int count = 0;
    while(count++ < 1000000) {
        cxx::con::coroutine::yield(p1);
    }
}

void perf_main(int argc, char** argv)
{
    cxx::con::coroutine* c = (cxx::con::coroutine* )argv[1];
    for(long i = 0; i < 5; ++i) {
        c->create(perf, (void* )i, 4096);
    }
}

TEST(coroutine, perfmance)
{
    // limit the thread to a CPU
    int cpu[5] = { 0, 1, 2, 3, 4 };
    int ret = cxx::sys::threadcontrol::policy(1, cpu + 1);

    cxx::con::coroutine c(256*1024);

    char** argv = new char*[2];
    argv[0] = new char[5];
    argv[1] = (char* )&c;
    strcpy(argv[0], "perf");

    c.start(perf_main, 2, argv);

    printf("perf done\n");
}

void delay(void *p)
{
    void* p1 = cxx::con::taskarg::p1(p);
    int ms = random() % 300;
    cxx::con::coroutine::delay(p1, ms);
    printf("delay %d (%d) is done\n", cxx::con::coroutine::id(p1), ms);
}

void delay_main(int argc, char** argv)
{
    srandom(time(NULL));
    cxx::con::coroutine* c = (cxx::con::coroutine* )argv[1];
    for(long i = 0; i < 3; ++i) {
        c->create(delay, (void* )i, 32684);
    }
}

TEST(task, delay)
{
    cxx::con::coroutine c(256*1024);

    char** argv = new char*[2];
    argv[0] = new char[5];
    argv[1] = (char* )&c;
    strcpy(argv[0], "delay");

    c.start(delay_main, 2, argv);

    printf("delay done\n");
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
