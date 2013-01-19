/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/str/trim.h"
#include <list>

TEST(Trim, left)
{
    std::string t1 = "abc";
    std::string t2 = "  abc";
    std::string t3 = "abc  ";
    ASSERT_EQ(cxx::str::trim_left(t1), "abc");
    ASSERT_EQ(cxx::str::trim_left(t2), "abc");
    ASSERT_EQ(cxx::str::trim_left(t3), "abc  ");
}

TEST(Trim, right)
{
    std::string t1 = "abc";
    std::string t2 = "  abc";
    std::string t3 = "abc  ";
    ASSERT_EQ(cxx::str::trim_right(t1), "abc");
    ASSERT_EQ(cxx::str::trim_right(t2), "  abc");
    ASSERT_EQ(cxx::str::trim_right(t3), "abc");
}

TEST(Trim, both)
{
    std::string t1 = "abc";
    std::string t2 = "  abc";
    std::string t3 = "abc  ";
    ASSERT_EQ(cxx::str::trim(t1), "abc");
    ASSERT_EQ(cxx::str::trim(t2), "abc");
    ASSERT_EQ(cxx::str::trim(t3), "abc");
}

bool is_dollar(char c)
{
    return c == '$';
}

TEST(Trim, symbol)
{
    std::string t1 = "abc";
    std::string t2 = "$$abc";
    std::string t3 = "abc$$";
    ASSERT_EQ(cxx::str::trim_if(t1, is_dollar), "abc");
    ASSERT_EQ(cxx::str::trim_if(t2, is_dollar), "abc");
    ASSERT_EQ(cxx::str::trim_if(t3, is_dollar), "abc");
}

TEST(Trim, list)
{
    std::list<char >    test;
    test.push_back(' ');
    test.push_back('a');
    test.push_back('b');
    test.push_back(' ');
    cxx::str::trim(test);
    ASSERT_EQ(test.size(), 2);
    ASSERT_EQ(*(test.begin()), 'a');
    ASSERT_EQ(*(++test.begin()), 'b');
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
