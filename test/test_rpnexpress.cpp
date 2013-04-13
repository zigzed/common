/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/alg/rpnexp.h"

class test_func : public cxx::alg::rpn_function {
public:
    virtual void print() = 0;
};

class function_func : public test_func {
public:
    function_func(const char* n) : n_(n) {}
    void push_arg(rpn_function *fn) {
        test_func* t = (test_func* )fn;
        a_.push_back(t);
    }
    void done_arg() {}
    void print() {
        if(a_.empty()) {
            printf("%s ", n_.c_str());
            return;
        }

        printf("%s(", n_.c_str());
        for(size_t i = 0; i < a_.size(); ++i) {
            a_[i]->print();
            if(i != a_.size() - 1)
                printf(",");
        }
        printf(") ");
    }
private:
    std::string n_;
    std::vector<test_func* > a_;
};

class test_symbol : public cxx::alg::rpn_symbols {
public:
    cxx::alg::rpn_function* resolve(const char *name) const {
        return new function_func(name);
    }
};

TEST(rpn_expr, usage)
{
    cxx::alg::rpn_express   exp;
    test_symbol             sym;
    exp.symbol(&sym);
    cxx::alg::rpn_express::expr_t ret = exp.parser("123 + 456");
    ASSERT_EQ(ret.size(), 3);
    ASSERT_EQ(ret[0].first, "123");
    ASSERT_EQ(ret[1].first, "456");
    ASSERT_EQ(ret[2].first, "+");
    for(size_t i = 0; i < ret.size(); ++i) {
        printf("%s ", ret[i].first.c_str());
    }
    printf("\n");
    cxx::alg::rpn_function* fn = exp.create(ret);
    ((test_func* )fn)->print();
    printf("\n");
}

TEST(rpn_expr, assignment)
{
    test_symbol             sym;
    cxx::alg::rpn_express   exp;
    try {
        exp.symbol(&sym);
        exp.setarg("D", 3);
        cxx::alg::rpn_express::expr_t ret = exp.parser("a = D(f - b * c + d, !e, g)");
        for(size_t i = 0; i < ret.size(); ++i) {
            printf("%s ", ret[i].first.c_str());
        }
        printf("\n");
        ASSERT_EQ(ret.size(), 13);
        ASSERT_EQ(ret[0].first, "a");
        ASSERT_EQ(ret[1].first, "f");
        ASSERT_EQ(ret[2].first, "b");
        ASSERT_EQ(ret[12].first, "=");

        cxx::alg::rpn_function* fn = exp.create(ret);
        ASSERT_TRUE(fn != NULL);
        ((test_func* )fn)->print();
        printf("\n");
    }
    catch(const cxx::alg::rpn_error& e) {
        printf("error: %d - %s, %d\n", e.err(), e.what(), e.pos());
    }
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
