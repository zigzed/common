/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/alg/variant.h"
#include <sstream>
#include <algorithm>

TEST(variant, usage)
{
    {
        cxx::alg::variant_t a(100);
        cxx::alg::variant_t b(100);
        cxx::alg::variant_t c = a + b;
        ASSERT_EQ(c.get_mtype(), cxx::alg::variant_t::Number);
        ASSERT_EQ(c.as_number(), 200);
    }
    {
        cxx::alg::variant_t a(100);
        cxx::alg::variant_t b(100);
        cxx::alg::variant_t c = a - b;
        ASSERT_EQ(c.get_mtype(), cxx::alg::variant_t::Number);
        ASSERT_EQ(c.as_number(), 0);
    }
    {
        cxx::alg::variant_t a(100);
        cxx::alg::variant_t b(100);
        cxx::alg::variant_t c = a * b;
        ASSERT_EQ(c.get_mtype(), cxx::alg::variant_t::Number);
        ASSERT_EQ(c.as_number(), 10000);
    }
    {
        cxx::alg::variant_t a(100);
        cxx::alg::variant_t b(100);
        cxx::alg::variant_t c = a / b;
        ASSERT_EQ(c.get_mtype(), cxx::alg::variant_t::Number);
        ASSERT_EQ(c.as_number(), 1);
    }
    {
        cxx::alg::variant_t a(100);
        cxx::alg::variant_t b(0);
        cxx::alg::variant_t c = a / b;
        ASSERT_EQ(c.get_mtype(), cxx::alg::variant_t::Nil);
    }
    {
        cxx::alg::variant_t a(100);
        cxx::alg::variant_t b("100");
        cxx::alg::variant_t c = a * b;
        ASSERT_EQ(c.get_mtype(), cxx::alg::variant_t::Number);
        ASSERT_EQ(c.as_number(), 10000);
    }
    {
        cxx::alg::variant_t a(100);
        cxx::alg::variant_t b("0x64");
        cxx::alg::variant_t c = a * b;
        ASSERT_EQ(c.get_mtype(), cxx::alg::variant_t::Number);
        ASSERT_EQ(c.as_number(), 10000);
    }
}

TEST(variant, string)
{
    cxx::alg::variant_t a(100);
    cxx::alg::variant_t b("0x64");
    ASSERT_EQ(a.as_number(), 100);
    ASSERT_EQ(a.as_string(), "100");
    ASSERT_FLOAT_EQ(a.as_double(), 100);
    ASSERT_EQ(b.as_number(), 100);
    ASSERT_EQ(b.as_string(), "0x64");
    ASSERT_FLOAT_EQ(a.as_double(), 100);
}

TEST(variant, performance)
{
    for(int i = 0; i < 1000000; ++i) {
        cxx::alg::variant_t a(i);
        cxx::alg::variant_t b(i * 2);
        cxx::alg::variant_t c = a + b;
        ASSERT_EQ(c.get_mtype(), cxx::alg::variant_t::Number);
        ASSERT_EQ(c.as_number(), 3 * i);
    }
}

TEST(variant, performance2)
{
    for(int i = 0; i < 1000000; ++i) {
        cxx::alg::variant_t a(i);
        cxx::alg::variant_t b("8");
        cxx::alg::variant_t c = a + b;
        ASSERT_EQ(c.get_mtype(), cxx::alg::variant_t::Number);
        ASSERT_EQ(c.as_number(), i + 8);
    }
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
