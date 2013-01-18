/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/datetime.h"

TEST(datetime, make)
{
    cxx::datetime date(2013, 1, 19, 0, 2, 1, 200);
    cxx::datetime::calendar c = cxx::datetime::calendar(date);

    ASSERT_EQ(c.year, 2013);
    ASSERT_EQ(c.mon,  1);
    ASSERT_EQ(c.day,  19);
    ASSERT_EQ(c.hour, 0);
    ASSERT_EQ(c.min,  2);
    ASSERT_EQ(c.sec,  1);
    ASSERT_EQ(c.msec, 200);
}

TEST(datetime, math)
{
    cxx::datetime date1(2013, 1, 19, 0, 2, 1, 200);
    cxx::datetime date2 = date1 - cxx::datetimespan(0, 0, 0, 1, 29);
    cxx::datetime::calendar c = cxx::datetime::calendar(date2);

    ASSERT_EQ(c.year, 2013);
    ASSERT_EQ(c.mon,  1);
    ASSERT_EQ(c.day,  19);
    ASSERT_EQ(c.hour, 0);
    ASSERT_EQ(c.min,  2);
    ASSERT_EQ(c.sec,  0);
    ASSERT_EQ(c.msec, 171);

    date2 -= cxx::datetimespan(0, 0, 0, 1, 100);
    c = cxx::datetime::calendar(date2);
    ASSERT_EQ(c.year, 2013);
    ASSERT_EQ(c.mon,  1);
    ASSERT_EQ(c.day,  19);
    ASSERT_EQ(c.hour, 0);
    ASSERT_EQ(c.min,  1);
    ASSERT_EQ(c.sec,  59);
    ASSERT_EQ(c.msec, 71);
}


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

