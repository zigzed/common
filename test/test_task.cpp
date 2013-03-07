/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/con/coroutine.h"
#include "common/con/channel.h"
#include "common/sys/threads.h"
#include "common/sys/cputimes.h"
#include "common/sys/mutex.h"
#include <sys/time.h>

void dice(cxx::con::coroutine* c, void* p)
{ 
    timeval tv;
    gettimeofday(&tv, NULL);
    srandom(tv.tv_usec);
    int sum = 0;
    while((sum += random() % 6 + 1) < 10000) {
        c->yield();
    }

    printf("winner is %d, %d\n", c->getid(), sum);
    c->sched()->quit();
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
    int count = 0;
    while(count++ < 1000000) {
        c->yield();
    }
    printf("%d done\n", c->getid());
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
    c->sleep(ms);
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

//typedef cxx::con::channel<int > chan;
typedef cxx::con::channel<int, cxx::sys::plainmutex > chan;
typedef cxx::con::channel<int, cxx::sys::spin_mutex > chan2;


void asleep(cxx::con::coroutine* c, void* p)
{
    chan* ch = (chan* )p;

    int x;
    bool r = ch->recv(c, x, 1000);
    printf("notified: %d, %d\n", x, r);
}

void posting(cxx::con::coroutine* c, void* p)
{
    chan* ch = (chan* )p;

    bool r = ch->send(c, 1);
    printf("waking up 1: %d\n", r);

    r = ch->send(c, 2);
    printf("waking up 2: %d\n", r);

    r = ch->send(c, 3);
    printf("waking up 3: %d\n", r);

    time_t a1 = time(NULL);
    r = ch->send(c, 4, 1000);
    time_t a2 = time(NULL);
    printf("waking up 4: %d, %d, %d, %d\n", r, a1, a2, a2 - a1);
    ASSERT_EQ(r, false);
    ASSERT_EQ(a2 - a1, 1);
}

void notify(cxx::con::coroutine* c, void* p)
{
    c->sched()->spawn(posting, p);
}

TEST(coroutine, wait_post)
{
    cxx::con::scheduler c;
    chan ch(1);

    for(long i = 1; i < 3; ++i) {
        c.spawn(asleep, &ch, 32768);
    }

    c.spawn(notify, &ch, 32768);
    c.start();

    printf("sleep done\n");
}

void sender(cxx::con::coroutine* c, void* p)
{
    chan* ch = (chan *)p;
    ch->send(c, c->getid());
    printf("sender: %d done\n", c->getid());
}

void receiver(cxx::con::coroutine* c, void* p)
{
    chan* ch = (chan* )p;
    int v;
    int i = 0;
    while(i++ < 10 && ch->recv(c, v)) {
        printf("received: %d, %d\n", i,  v);
    }
}

TEST(coroutine, channel)
{
    cxx::con::scheduler c;
    chan    ch(3);

    for(long i = 0; i < 10; ++i) {
        c.spawn(sender, &ch);
    }

    c.spawn(receiver, &ch);
    c.start();

    printf("channel done\n");
}

//void sender_to(cxx::con::coroutine* c, void* p)
//{
//    chan* ch = (chan *)p;
//    bool r = ch->send(c, c->getid(), 100);
//    printf("sender: %d done: %s\n", c->getid(), r ? "ok" : "failed");
//}

//void receiver_to(cxx::con::coroutine* c, void* p)
//{
//    chan* ch = (chan* )p;
//    int v;
//    int i = 0;

//    while(i++ < 10 && ch->recv(c, v, 100)) {
//        printf("received: %d, %d\n", i,  v);
//        c->delay(50);
//    }
//}

//TEST(coroutine, channel_timeout)
//{
//    cxx::con::scheduler c;
//    chan    ch(1);

//    for(long i = 0; i < 10; ++i) {
//        c.spawn(sender_to, &ch);
//    }

//    c.spawn(receiver_to, &ch);
//    c.start();

//    printf("channel timeout done\n");
//}

void perf_send(cxx::con::coroutine* c, void* p)
{
    chan* ch = (chan *)p;
    for(size_t i = 0; i < 1000000; ++i) {
        ch->send(c, i);
    }
}

void perf_recv(cxx::con::coroutine* c, void *p)
{
    chan* ch = (chan *)p;
    int x = -1;
    for(size_t i = 0; i < 1000000; ++i) {
        ch->recv(c, x);
        ASSERT_EQ(x, i);
    }
    ASSERT_EQ(x, 1000000-1);
}

TEST(coroutine, performance_chan)
{
    cxx::con::scheduler c;
    chan    ch(256);

    c.spawn(perf_send, &ch);
    c.spawn(perf_recv, &ch);

    cxx::sys::cpu_times usage;

    c.start();

    //printf("perf chan: %s\n", usage.report().c_str());
}

void perf_send2(cxx::con::coroutine* c, void* p)
{
    chan2* ch = (chan2 *)p;
    for(size_t i = 0; i < 1000000; ++i) {
        ch->send(c, i);
    }
}

void perf_recv2(cxx::con::coroutine* c, void *p)
{
    chan2* ch = (chan2 *)p;
    int x = -1;
    for(size_t i = 0; i < 1000000; ++i) {
        ch->recv(c, x);
        ASSERT_EQ(x, i);
    }
    ASSERT_EQ(x, 1000000-1);
}

TEST(coroutine, performance_chan2)
{
    cxx::con::scheduler c;
    chan2    ch(256);

    c.spawn(perf_send2, &ch);
    c.spawn(perf_recv2, &ch);

    cxx::sys::cpu_times usage;

    c.start();

    //printf("perf chan: %s\n", usage.report().c_str());
}

//TEST(coroutine, performance_chan_thread)
//{
//    cxx::con::scheduler_group c(2);
//    chan    ch1(256);

//    c[0]->spawn(perf_send, &ch1);
//    c[1]->spawn(perf_recv, &ch1);

//    cxx::sys::cpu_times usage;

//    c.start();

//    //printf("perf thread chan: %s\n", usage.report().c_str());
//}

//void perf_recv3(cxx::con::coroutine* c, void *p)
//{
//    chan* ch = (chan *)p;
//    int x = -1;
//    for(size_t i = 0; i < 2000000; ++i) {
//        ch->recv(c, x);
//    }
//    ASSERT_EQ(x, 1000000-1);
//}

//TEST(coroutine, chan_mpsc)
//{
//    cxx::con::scheduler_group c(3);
//    chan    ch1(256);

//    c[0]->spawn(perf_send, &ch1);
//    c[1]->spawn(perf_send, &ch1);
//    c[2]->spawn(perf_recv3, &ch1);

//    cxx::sys::cpu_times usage;

//    c.start();

//    //printf("perf thread chan: %s\n", usage.report().c_str());
//}

//TEST(coroutine, chan_mpsc_s)
//{
//    cxx::con::scheduler_group c(3);
//    chan    ch1(256);

//    c[0]->spawn(perf_send, &ch1);
//    c[0]->spawn(perf_send, &ch1);
//    c[2]->spawn(perf_recv3, &ch1);

//    cxx::sys::cpu_times usage;

//    c.start();

//    //printf("perf thread chan: %s\n", usage.report().c_str());
//}


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
