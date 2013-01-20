/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/sys/cputimes.h"
#include "common/sys/threads.h"
#include <cmath>

TEST(CPU_TIMES, simple)
{
    cxx::sys::cpu_times timer;
    cxx::sys::threadcontrol::sleep(100);
    timer.stop();
    cxx::sys::cpu_times::times usage = timer.elapsed();

    double sec = 1000000000.0L;
    printf("wall: %f total: %f user: %f sys: %f usage: %f\%\n",
           usage.wall / sec, (usage.user + usage.sys) / sec,
           usage.user / sec, usage.sys / sec, ((usage.user + usage.sys) * 100.0 / usage.wall));

    timer.start();
    for(int j = 0; j < 100; ++j) {
        for(int i = 0; i < 1000000; ++i) {
            std::sqrt(123.456L);
        }
    }
    timer.stop();
    usage = timer.elapsed();
    printf("wall: %f total: %f user: %f sys: %f usage: %f\%\n",
           usage.wall / sec, (usage.user + usage.sys) / sec,
           usage.user / sec, usage.sys / sec, ((usage.user + usage.sys) * 100.0 / usage.wall));
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
