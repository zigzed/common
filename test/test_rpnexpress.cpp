/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/alg/rpnexp.h"

class test_func : public cxx::alg::rpn_function {
public:
    virtual void print() = 0;
    virtual int  compute() { return 0; }
};

class assign_func : public test_func {
public:
    ~assign_func() {
        for(size_t i = 0; i < a_.size(); ++i) {
            delete a_[i];
        }
    }
    assign_func(const cxx::alg::rpn_function::arg_t& args) {
        for(size_t i = 0; i < args.size(); ++i)
            a_.push_back((test_func* )args[i]);
    }
    void print() {
        a_[0]->print();
        printf("=");
        a_[1]->print();
    }
    int compute() {
        return a_[1]->compute();
    }
private:
    std::vector<test_func* >    a_;
};

class algo_plus_func : public test_func {
public:
    ~algo_plus_func() {
        for(size_t i = 0; i < a_.size(); ++i) {
            delete a_[i];
        }
    }
    algo_plus_func(const cxx::alg::rpn_function::arg_t& args) {
        for(size_t i = 0; i < args.size(); ++i)
            a_.push_back((test_func* )args[i]);
    }
    void print() {
        printf("(");
        a_[0]->print();
        printf("+");
        a_[1]->print();
        printf(")");
    }
    int compute() {
        return a_[0]->compute() + a_[1]->compute();
    }
private:
    std::vector<test_func* >    a_;
};

class algo_minus_func : public test_func {
public:
    ~algo_minus_func() {
        for(size_t i = 0; i < a_.size(); ++i) {
            delete a_[i];
        }
    }
    algo_minus_func(const cxx::alg::rpn_function::arg_t& args) {
        for(size_t i = 0; i < args.size(); ++i)
            a_.push_back((test_func* )args[i]);
    }
    void print() {
        printf("(");
        a_[0]->print();
        printf("-");
        a_[1]->print();
        printf(")");
    }
    int compute() {
        return a_[0]->compute() - a_[1]->compute();
    }
private:
    std::vector<test_func* >    a_;
};

class algo_mul_func : public test_func {
public:
    ~algo_mul_func() {
        for(size_t i = 0; i < a_.size(); ++i) {
            delete a_[i];
        }
    }
    algo_mul_func(const cxx::alg::rpn_function::arg_t& args) {
        for(size_t i = 0; i < args.size(); ++i)
            a_.push_back((test_func* )args[i]);
    }
    void print() {
        printf("(");
        a_[0]->print();
        printf("*");
        a_[1]->print();
        printf(")");
    }
    int compute() {
        return a_[0]->compute() * a_[1]->compute();
    }
private:
    std::vector<test_func* >    a_;
};

class const_value_func : public test_func {
public:
    const_value_func(const char* n) : v_(0) {
        v_ = atol(n);
    }
    void print() {
        printf("%d", v_);
    }
    int compute() {
        return v_;
    }
private:
    int v_;
};

class function_func : public test_func {
public:
    function_func(const char* n, const cxx::alg::rpn_function::arg_t& args) : n_(n) {
        for(size_t i = 0; i < args.size(); ++i)
            a_.push_back((test_func* )args[i]);
    }
    void print() {
        if(a_.empty()) {
            printf("%s", n_.c_str());
            return;
        }

        printf("%s(", n_.c_str());
        for(size_t i = 0; i < a_.size(); ++i) {
            a_[i]->print();
            if(i != a_.size() - 1)
                printf(",");
        }
        printf(")");
    }
private:
    std::string n_;
    std::vector<test_func* > a_;
};

static bool is_number(const char* n) {
    const char* p = n;
    while(*p) {
        if(!isdigit(*p))
            return false;
        p++;
    }
    return true;
}

class test_symbol : public cxx::alg::rpn_symbols {
public:
    cxx::alg::rpn_function* resolve(const char *name, const cxx::alg::rpn_function::arg_t& args) const {
        return new function_func(name, args);
    }
};

class test2_symbol : public cxx::alg::rpn_symbols {
public:
    cxx::alg::rpn_function* resolve(const char *name, const cxx::alg::rpn_function::arg_t& args) const {
        if(is_number(name)) {
            return new const_value_func(name);
        }
        else if(strcmp(name, "+") == 0) {
            return new algo_plus_func(args);
        }
        else if(strcmp(name, "-") == 0) {
            return new algo_minus_func(args);
        }
        else if(strcmp(name, "*") == 0) {
            return new algo_mul_func(args);
        }
        return NULL;
    }
};

TEST(rpn_expr, usage)
{
    test2_symbol            sym;
    cxx::alg::rpn_express   exp;
    exp.lookup(&sym);
    cxx::alg::rpn_function* fn = exp.create("123+456");
    ((test_func* )fn)->print();
    printf("=");
    int r = ((test_func* )fn)->compute();
    ASSERT_EQ(r, 579);
    printf("%d\n", r);
    printf("\n");


    cxx::alg::rpn_function* fn2 = exp.create("234 * (1 + 5) - 8");
    ((test_func* )fn2)->print();
    printf("=");
    int r2 = ((test_func* )fn2)->compute();
    ASSERT_EQ(r2, 1396);
    printf("%d\n", r2);
    printf("\n");
}

TEST(rpn_expr, assignment)
{
    test_symbol             sym;
    cxx::alg::rpn_express   exp;
    try {
        exp.lookup(&sym);
        exp.setarg("D", 3);

        cxx::alg::rpn_function* fn = exp.create("a = D(f - b * c + d, !e, g)");
        ASSERT_TRUE(fn != NULL);
        ((test_func* )fn)->print();
        printf("\n");
    }
    catch(const cxx::alg::rpn_error& e) {
        printf("error: %d - %s, %d\n", e.err(), e.what(), e.pos());
    }
}

TEST(rpn_expr, expr)
{
    test_symbol             sym;
    cxx::alg::rpn_express   exp;
    try {
        exp.lookup(&sym);
        exp.setarg("fn_1", 3);
        exp.setarg("fn_2", 2);

        cxx::alg::rpn_function* fn = exp.create("x = fn_1(1 + fn_2(3, 5), 6, 7*(8+1)) && true || !m");
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
