/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/con/coroutine.h"
#include "common/sys/threads.h"
#include <sys/time.h>

void dice(cxx::con::coroutine* c, void* p)
{ 
    c->name("dice %d", c->getid());

    timeval tv;
    gettimeofday(&tv, NULL);
    srandom(tv.tv_usec);
    int sum = 0;
    while((sum += random() % 6 + 1) < 10000) {
        c->yield();
    }

    printf("winner is %s %d, %d\n", c->name(), c->getid(), sum);
    c->sched()->quit(0);
}

TEST(coroutine, dice)
{
    cxx::con::scheduler c;

    for(long i = 0; i < 5; ++i) {
        c.spawn(dice, (void* )i, cxx::con::stack::minimum_size());
    }

    c.start();

    printf("dice done\n");
}

void perf(cxx::con::coroutine* c, void* p)
{
    c->name("perf task %d", c->getid());
    int count = 0;
    while(count++ < 1000000) {
        c->yield();
    }
    printf("%s done\n", c->name());
}


TEST(coroutine, perfmance)
{
    // limit the thread to a CPU
    int cpu[5] = { 0, 1, 2, 3, 4 };
    int ret = cxx::sys::threadcontrol::policy(1, cpu + 1);

    cxx::con::scheduler c;

    for(long i = 0; i < 5; ++i) {
        c.spawn(perf, (void* )i, cxx::con::stack::minimum_size());
    }

    c.start();

    printf("perf done\n");
}

void delay(cxx::con::coroutine* c, void *p)
{
    int ms = random() % 300;
    c->delay(ms);
    printf("delay %d (%d) is done\n", c->getid(), ms);
}

TEST(coroutine, delay)
{
    cxx::con::scheduler c;

    for(long i = 0; i < 3; ++i) {
        c.spawn(delay, (void* )i, cxx::con::stack::default_size());
    }

    c.start();

    printf("delay done\n");
}

void asleep(cxx::con::coroutine* c, void* p)
{
    if((long )p != 1) {
        c->sched()->spawn(asleep, (void* )1, 32768);
    }

    printf("waiting for: %d\n", (long)p);

    c->sched()->wait((long)p);

    printf("notified: %d\n", (long)p);
}

void posting(cxx::con::coroutine* c, void* p)
{
    printf("posting\n");

    int r = c->sched()->post(1, 1);
    printf("waking up 1: %d\n", r);
    ASSERT_EQ(r, 2);

    r = c->sched()->post(2, 0);
    printf("waking up 2: %d\n", r);
    ASSERT_EQ(r, 1);

    r = c->sched()->post(3, 1);
    printf("waking up 3: %d\n", r);
    ASSERT_EQ(r, 0);
}

void notify(cxx::con::coroutine* c, void* p)
{
    //c->yield();

    c->sched()->spawn(posting, NULL, 32768);
}

TEST(coroutine, wait_post)
{
    cxx::con::scheduler c;

    for(long i = 1; i < 3; ++i) {
        c.spawn(asleep, (void* )i, 32768);
    }

    c.spawn(notify, NULL, 32768);
    c.start();

    printf("sleep done\n");
}


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
