/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/sys/coroutine.h"
#include "common/sys/threads.h"
#include <sys/time.h>
#include <errno.h>

void dice(void* t, void *arg)
{
    timeval tv;
    gettimeofday(&tv, NULL);
    srandom(tv.tv_usec);
    int sum = 0;
    while((sum += random() % 6 + 1) < 10000)
        cxx::sys::coroutine::yield(t);

    printf("winner is %d, %d\n", cxx::sys::coroutine::id(t), sum);
    //cxx::sys::coroutine::quit(t, 0);
}

void dice_main(int argc, char** argv)
{
    cxx::sys::coroutine* c;
    c = (cxx::sys::coroutine* )argv[1];
    for(int i = 0; i < 1000; ++i) {
        c->create(dice, NULL, 2048);
    }
}

TEST(coroutine, dice)
{
    cxx::sys::coroutine c;

    char** argv = new char*[2];
    argv[0] = new char[5];
    argv[1] = (char* )&c;
    strcpy(argv[0], "dice");

    c.start(dice_main, 1, argv);

    printf("dice done\n");
}


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
