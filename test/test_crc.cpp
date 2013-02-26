/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/alg/crc.h"
#include "common/sys/cputimes.h"

TEST(CRC, LRC08)
{
    cxx::alg::lrc08     tst1;
    const unsigned char set1[] = "\x02\x30\x30\x31\x23\x03";
    unsigned char   sum1 = tst1.sum(set1, 6);
    ASSERT_EQ(sum1, 0x47);

    cxx::sys::cpu_times timer;
    for(int i = 0; i < 1000000; ++i) {
        unsigned char x = tst1.sum(&i, sizeof(i));
    }

    timer.stop();
    cxx::sys::cpu_times::times usage = timer.elapsed();
    double sec = 1000000000.0L;
    printf("wall: %f total: %f user: %f sys: %f usage: %f\%\n",
           usage.wall / sec, (usage.user + usage.sys) / sec,
           usage.user / sec, usage.sys / sec, ((usage.user + usage.sys) * 100.0 / usage.wall));
}

TEST(CRC, crc32)
{
    cxx::alg::crc32 tst1;

    cxx::sys::cpu_times timer;
    for(int i = 0; i < 1000000; ++i) {
        unsigned int x = tst1.sum(&i, sizeof(i));
    }

    timer.stop();
    cxx::sys::cpu_times::times usage = timer.elapsed();
    double sec = 1000000000.0L;
    printf("wall: %f total: %f user: %f sys: %f usage: %f\%\n",
           usage.wall / sec, (usage.user + usage.sys) / sec,
           usage.user / sec, usage.sys / sec, ((usage.user + usage.sys) * 100.0 / usage.wall));
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
