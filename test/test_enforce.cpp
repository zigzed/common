/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/sys/error.h"

TEST(enforce, usage)
{
    int e = cxx::sys::err::get();
    ENFORCE(e == 0)(e);

    cxx::sys::err::set(1);
    e = cxx::sys::err::get();
    ENFORCE(e == 0)(e)(cxx::sys::err::get());
}



int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
