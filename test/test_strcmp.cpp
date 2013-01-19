/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/str/compare.h"

TEST(strcmp, iseq)
{
    ASSERT_EQ(cxx::str::is_equal<std::string >()("abc", "abc"), true);
    ASSERT_EQ(cxx::str::is_equal<std::string >()("abc", "cba"), false);
}

TEST(strcmp, iseq_ic)
{
    try {
        ASSERT_EQ(cxx::str::is_case_equal<std::string >()("abc", "aBc"), true);
        ASSERT_EQ(cxx::str::is_case_equal<std::string >()("abc", "aBC"), true);
        ASSERT_EQ(cxx::str::is_case_equal<std::string >()("abc", "ABC"), true);
        ASSERT_EQ(cxx::str::is_case_equal<std::string >()("abc", "abc"), true);
        ASSERT_EQ(cxx::str::is_case_equal<std::string >()("abc", "cba"), false);
    }
    catch(const std::bad_cast& e) {
        printf("bad_cast: %s\n", e.what());
    }
}

TEST(strcmp, isless)
{
    ASSERT_EQ(cxx::str::is_less<std::string >()("abc", "xyz"), true);
    ASSERT_EQ(cxx::str::is_less<std::string >()("abc", "ABC"), false);
    ASSERT_EQ(cxx::str::is_less<std::string >()("xyz", "abc"), false);
}

TEST(strcmp, isless_ic)
{
    ASSERT_EQ(cxx::str::is_case_less<std::string >()("abc", "xyz"), true);
    ASSERT_EQ(cxx::str::is_case_less<std::string >()("abc", "ABC"), false);
    ASSERT_EQ(cxx::str::is_case_less<std::string >()("xyz", "abc"), false);
}


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
