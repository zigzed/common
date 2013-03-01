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

    timeval tv;
    gettimeofday(&tv, NULL);
    srandom(tv.tv_usec);
    int sum = 0;
    while((sum += random() % 6 + 1) < 10000) {
        cxx::sys::coroutine::yield(p1);
    }

    printf("winner is %d, %d\n", cxx::sys::coroutine::id(p1), sum);
    cxx::sys::coroutine::quit(p1, 0);
}

void dice_main(int argc, char** argv)
{
    cxx::sys::coroutine* c = (cxx::sys::coroutine* )argv[1];
    for(long i = 0; i < 5; ++i) {
        c->create(dice, (void* )i, 32684);
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


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
