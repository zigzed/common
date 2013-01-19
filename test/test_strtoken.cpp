/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include <stdio.h>
#include <cassert>
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/str/tokenizer.h"


TEST(tokenizer, simple)
{
    {
        cxx::str::tokenizer    token("a\t\t   b", "\t");
        size_t s = token.size();
        ASSERT_EQ(s, 3);
    }
    {
        cxx::str::tokenizer    token("a\t\t   b", "\t", true);
        size_t s = token.size();
        ASSERT_EQ(s, 2);
    }
    {
        cxx::str::tokenizer    token("a\t\t   b", "\t", false);
        size_t s = token.size();
        ASSERT_EQ(s, 3);
    }
    {
        cxx::str::tokenizer    token("a.b.c", ".");
        size_t s = token.size();
        ASSERT_EQ(s, 3);
    }
    {
        cxx::str::tokenizer    token("a", ".");
        size_t s = token.size();
        ASSERT_EQ(s, 1);
    }
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
