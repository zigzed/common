/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/lua/scriptvm.h"
#include "common/lua/scriptbind.h"
#include "common/lua/scriptcall.h"
#include "common/gtest/gtest.h"
#include <iostream>
#include <ctime>

struct A {
};

void  f00()
{
}

void  f01(int a)
{
}

void  f02(int a, float b)
{
}

void  f03(int a, float b, const char* c)
{
}

void  f04(int a, float b, const char* c, A* d)
{
}

float f10()
{
    return 3.14f;
}

float f11(int a)
{
    return 3.14f;
}

float f12(int a, float b)
{
    return 3.14f;
}

float f13(int a, float b, const char* c)
{
    return 3.14f;
}

float f14(int a, float b, const char* c, A* d)
{
    return 3.14f;
}

class C01 {
public:
    C01() {}
    C01(int a) {}
    C01(int a, float b) {}
    C01(int a, float b, const char* c) {}
    C01(int a, float b, const char* c, A* d) {}
};

TEST(luabench, bench)
{
    cxx::lua::ScriptVM	vm(cxx::lua::ScriptVM::LIB_STRING |
                           //cxx::lua::ScriptVM::LIB_IO |
                           cxx::lua::ScriptVM::LIB_DEBUG);

    cxx::lua::module_(&vm)
        .class_<A >("A")
            .create(cxx::lua::constructor<A >());
    cxx::lua::module_(&vm)
        .function("f00",	&f00)
        .function("f01",	&f01)
        .function("f02",	&f02)
        .function("f03",	&f03)
        .function("f04",	&f04)
        .function("f10",	&f10)
        .function("f11",	&f11)
        .function("f12",	&f12)
        .function("f13",	&f13)
        .function("f14",	&f14)
    ;

    clock_t	total 		= 0;
    int 	loops 		= 10;
    int 	num_calls 	= 10000;
    double	diff		= 0;

    cxx::lua::ScriptGC* gc = vm.collector();

    gc->collect();
    std::cout << "before run: " << gc->usedmem() << "kBytes\n";

    vm.interpret("function test1()\n"
                 "	return false\n"
                 "end\n"
                 "function test2()\n"
                 "	return true\n"
                 "end\n"
                 "function test3()\n"
                 "	return 0\n"
                 "end\n"
                 "function test4()\n"
                 "	return 1\n"
                 "end\n"
                 , -1);
    int  x1 = cxx::lua::script<int >::call(&vm, "test1");
    ASSERT_EQ(x1, 0);
    int  x2 = cxx::lua::script<int >::call(&vm, "test2");
    ASSERT_EQ(x2, 0);
    int  x3 = cxx::lua::script<int >::call(&vm, "test3");
    ASSERT_EQ(x3, 0);
    int  x4 = cxx::lua::script<int >::call(&vm, "test4");
    ASSERT_EQ(x4, 1);

    bool a1 = cxx::lua::script<bool >::call(&vm, "test1");
    ASSERT_EQ(a1, false);
    bool a2 = cxx::lua::script<bool >::call(&vm, "test2");
    ASSERT_EQ(a2, true);
    bool a3 = cxx::lua::script<bool >::call(&vm, "test3");
    ASSERT_EQ(a3, false);
    bool a4 = cxx::lua::script<bool >::call(&vm, "test4");
    ASSERT_EQ(a4, true);

    for (int i = 0; i < loops; ++i) {
        std::clock_t begin1 = std::clock();
        vm.interpret("for i = 1, 10000 do\n"
                     "  f00()\n"
                     "end", -1);
        std::clock_t end1 = std::clock();
        total += (end1 - begin1);
    }
    diff = double(total) / (double)CLOCKS_PER_SEC;

    std::cout << "benchmark f00: " << diff * 10000 / num_calls / loops << "microsecond per call"
        << "  memory: " << gc->usedmem() << "KBytes\n";
    gc->collect();
    total = 0;

    std::cout << "before run: " << gc->usedmem() << "kBytes\n";

    for (int i = 0; i < loops; ++i) {
        std::clock_t begin1 = std::clock();
        vm.interpret("for i = 1, 10000 do\n"
                     "  f01(1)\n"
                     "end", -1);
        std::clock_t end1 = std::clock();
        total += (end1 - begin1);
    }
    diff = double(total) / (double)CLOCKS_PER_SEC;

    std::cout << "benchmark f01: " << diff * 10000 / num_calls / loops << "microsecond per call"
        << "  memory: " << gc->usedmem() << "KBytes\n";
    gc->collect();
    total = 0;

    std::cout << "before run: " << gc->usedmem() << "kBytes\n";

    for (int i = 0; i < loops; ++i) {
        std::clock_t begin1 = std::clock();
        vm.interpret("for i = 1, 10000 do\n"
                     "  f02(1, 2.0)\n"
                     "end", -1);
        std::clock_t end1 = std::clock();
        total += (end1 - begin1);
    }
    diff = double(total) / (double)CLOCKS_PER_SEC;

    std::cout << "benchmark f02: " << diff * 10000 / num_calls / loops << "microsecond per call"
        << "  memory: " << gc->usedmem() << "KBytes\n";
    gc->collect();
    total = 0;

    std::cout << "before run: " << gc->usedmem() << "kBytes\n";

    for (int i = 0; i < loops; ++i) {
        std::clock_t begin1 = std::clock();
        vm.interpret("for i = 1, 10000 do\n"
                     "  f03(1, 2.0, 'abc')\n"
                     "end", -1);
        std::clock_t end1 = std::clock();
        total += (end1 - begin1);
    }
    diff = double(total) / (double)CLOCKS_PER_SEC;

    std::cout << "benchmark f03: " << diff * 10000 / num_calls / loops << "microsecond per call"
        << "  memory: " << gc->usedmem() << "KBytes\n";
    gc->collect();
    total = 0;

    std::cout << "before run: " << gc->usedmem() << "kBytes\n";

    for (int i = 0; i < loops; ++i) {
        clock_t begin1 = std::clock();
        vm.interpret("a = A()\n"
                     "for i = 1, 10000 do\n"
                     "  f04(1, 2.0, 'abc', a)\n"
                     "end", -1);
        clock_t end1 = std::clock();
        total += (end1 - begin1);
    }
    diff = double(total) / (double)CLOCKS_PER_SEC;

    std::cout << "benchmark f04: " << diff * 10000 / num_calls / loops << "microsecond per call"
        << "  memory: " << gc->usedmem() << "KBytes\n";
    gc->collect();
    total = 0;

    gc->collect();
    std::cout << "before run: " << gc->usedmem() << "kBytes\n";

    for (int i = 0; i < loops; ++i) {
        std::clock_t begin1 = std::clock();
        vm.interpret("for i = 1, 10000 do\n"
                     "  f10()\n"
                     "end", -1);
        std::clock_t end1 = std::clock();
        total += (end1 - begin1);
    }
    diff = double(total) / (double)CLOCKS_PER_SEC;

    std::cout << "benchmark f10: " << diff * 10000 / num_calls / loops << "microsecond per call"
        << "  memory: " << gc->usedmem() << "KBytes\n";
    gc->collect();
    total = 0;

    std::cout << "before run: " << gc->usedmem() << "kBytes\n";

    for (int i = 0; i < loops; ++i) {
        std::clock_t begin1 = std::clock();
        vm.interpret("for i = 1, 10000 do\n"

                     "  f11(1)\n"
                     "end", -1);
        std::clock_t end1 = std::clock();
        total += (end1 - begin1);
    }
    diff = double(total) / (double)CLOCKS_PER_SEC;

    std::cout << "benchmark f11: " << diff * 10000 / num_calls / loops << "microsecond per call"
        << "  memory: " << gc->usedmem() << "KBytes\n";
    gc->collect();
    total = 0;

    std::cout << "before run: " << gc->usedmem() << "kBytes\n";

    for (int i = 0; i < loops; ++i) {
        std::clock_t begin1 = std::clock();
        vm.interpret("for i = 1, 10000 do\n"
                     "  f12(1, 2.0)\n"
                     "end", -1);
        std::clock_t end1 = std::clock();
        total += (end1 - begin1);
    }
    diff = double(total) / (double)CLOCKS_PER_SEC;

    std::cout << "benchmark f12: " << diff * 10000 / num_calls / loops << "microsecond per call"
        << "  memory: " << gc->usedmem() << "KBytes\n";
    gc->collect();
    total = 0;

    std::cout << "before run: " << gc->usedmem() << "kBytes\n";

    for (int i = 0; i < loops; ++i) {
        std::clock_t begin1 = std::clock();
        vm.interpret("for i = 1, 10000 do\n"
                     "  f13(1, 2.0, 'abc')\n"
                     "end", -1);
        std::clock_t end1 = std::clock();
        total += (end1 - begin1);
    }
    diff = double(total) / (double)CLOCKS_PER_SEC;

    std::cout << "benchmark f13: " << diff * 10000 / num_calls / loops << "microsecond per call"
        << "  memory: " << gc->usedmem() << "KBytes\n";
    gc->collect();
    total = 0;

    std::cout << "before run: " << gc->usedmem() << "kBytes\n";

    for (int i = 0; i < loops; ++i) {
        clock_t begin1 = std::clock();
        vm.interpret("a = A()\n"
                     "for i = 1, 10000 do\n"
                     "  f14(1, 2.0, 'abc', a)\n"
                     "end", -1);
        clock_t end1 = std::clock();
        total += (end1 - begin1);
    }
    diff = double(total) / (double)CLOCKS_PER_SEC;

    std::cout << "benchmark f14: " << diff * 10000 / num_calls / loops << "microsecond per call"
        << "  memory: " << gc->usedmem() << "KBytes\n";
    gc->collect();
    total = 0;
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

