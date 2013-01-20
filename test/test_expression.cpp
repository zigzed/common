/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/alg/expression.h"

TEST(Expr, simple)
{
    cxx::alg::expression expr;
    ASSERT_EQ(5, expr.evaluate("5"));
    ASSERT_EQ(10, expr.evaluate("5 + 5"));
    ASSERT_EQ(10, expr.evaluate("1+2  +3+4"));
    ASSERT_EQ(10, expr.evaluate("100 - 90"));
    ASSERT_EQ(10, expr.evaluate("100 - 95 + 5"));
    ASSERT_EQ(10, expr.evaluate("2*5"));
    ASSERT_EQ(21, expr.evaluate("1+4 * 5"));
    ASSERT_EQ(16, expr.evaluate("1+4 * 5 - 5"));
    ASSERT_EQ(10, expr.evaluate("100 / 10"));
    ASSERT_EQ(10, expr.evaluate("2 + 32 / 4"));
    ASSERT_EQ(1,  expr.evaluate("8 % 7"));
}

TEST(Expr, parenthesis)
{
    cxx::alg::expression expr;
    ASSERT_EQ(1, expr.evaluate("(1)"));
    ASSERT_EQ(20, expr.evaluate("(2 + 2)*5"));
    ASSERT_EQ(20, expr.evaluate("(2 * (1 + 1)) * 5"));
}

TEST(Expr, variable)
{
    cxx::alg::expression expr;
    expr.variable("a", 3);
    ASSERT_EQ(3, expr.evaluate("1 * a"));
    ASSERT_EQ(1, expr.evaluate("3 / a"));
    ASSERT_EQ(1, expr.evaluate("a / 3"));
    ASSERT_EQ(7, expr.evaluate("4 + a"));

    ASSERT_EQ(7, expr.evaluate("b = 4 * 2 - 1"));
    ASSERT_EQ(14, expr.evaluate("b * 2"));

    ASSERT_EQ(14, expr.evaluate("a.b = 14"));
    ASSERT_EQ(28, expr.evaluate("a.b * 2"));
}

class test_resolver : public cxx::alg::resolver {
public:
    bool get(const std::string &name, cxx::alg::expression::value_type &value) {
        if(name == "a.b") {
            value = 1;
            return true;
        }
        else if(name == "a.c") {
            value = 2;
            return true;
        }
        return false;
    }
};

TEST(Expr, external)
{
    test_resolver           reso;
    cxx::alg::expression    expr;
    expr.external(&reso);
    ASSERT_EQ(14, expr.evaluate("a.b = 14"));
    ASSERT_EQ(14, expr.evaluate("a.c * 7"));
}

TEST(Expr, complex)
{
    cxx::alg::expression expr;
    ASSERT_EQ(expr.evaluate("1+2*(3-4)+5"), 4);
    ASSERT_EQ(expr.evaluate("1+2*(3-4)-5"), -6);
}

TEST(Expr, performance)
{
    cxx::alg::expression expr;
    for(int i = 0; i < 1000000; ++i) {
        expr.variable("a", i);
        expr.evaluate("a - 160");
    }
}

TEST(Expr, error)
{
    cxx::alg::expression expr1;
    cxx::alg::expression expr2;
    try {
        expr1.err_type(cxx::alg::expression::ET_EXCEPTION);
        expr1.evaluate("a + 2");
    }
    catch(const std::string& e) {
        printf("error: %s\n", e.c_str());
    }

    expr2.err_type(cxx::alg::expression::ET_ERRNO);
    expr2.evaluate("a + 2");
    if(!expr2.error().empty()) {
        printf("error: %s\n", expr2.error().c_str());
    }
}

TEST(Expr, allow_digit)
{
    cxx::alg::expression expr;
    ASSERT_EQ(14, expr.evaluate("a.b.c_1 = 14"));
    ASSERT_EQ(28, expr.evaluate("a.b.c_1 * 2"));
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
