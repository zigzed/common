/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/lua/scriptvm.h"
#include "common/lua/scriptbind.h"
#include "common/lua/scriptcall.h"
#include "common/gtest/gtest.h"
#include <iostream>

class TestA {
public:
    TestA() : a_(1), id_(++theid) {}
    TestA(int a) : a_(a), id_(++theid) {}
    ~TestA() { std::cout << "destructor with id = " << id_ << "\n"; }
    int output() { std::cout << a_ << "\n"; return 0; }
    int add() { std::cout << ++a_ << "\n"; return 0; }
    void nothing() { std::cout << "nothing\n"; }
    void nothing2(int a) { std::cout << "nothing 2: " << a << "\n"; }
private:
    int a_;
    int id_;
    static int theid;
};

int TestA::theid = 0;

struct TestS {
    int a;
    int b;
};

class TestVB {
public:
    static TestVB* create();
    virtual ~TestVB() { std::cout << "TestVB::~TestVB\n"; }
    virtual int do_something() const { return 1; }
};

class TestVI : public TestVB {
public:
    ~TestVI() { std::cout << "TestVI::~TestVI\n"; }
    int do_something() const { return 100; }
};

TestVB* TestVB::create()
{
    return new TestVI();
}

TestVB* make_TestVB()
{
    return TestVB::create();
}

void test_struct(TestS& ts)
{
    std::cout << "a=" << ts.a << " b=" << ts.b << "\n";
    ts.a = ts.a + 1;
    ts.b = ts.b - 1;
}

int	test1()
{
    std::cout << "test1\n";
    return 0;
}

int test2()
{
    std::cout << "test2\n";
    return 0;
}

void test3(int a)
{
    std::cout << "test3(" << a << ")\n";
}

void para0()
{
    std::cout << "para0()\n";
}

void para1(int a)
{
    std::cout << "para1(" << a << ")\n";
}

void para2(int a, int b)
{
    std::cout << "para1(" << a << "," << b << ")\n";
}


void para3(int a, int b, int c)
{
    std::cout << "para1(" << a << "," << b << "," << c << ")\n";
}


void para4(int a, int b, int c, int d)
{
    std::cout << "para1(" << a << "," << b << "," << c << "," << d << ")\n";
}


void para5(int a, int b, int c, int d, int e)
{
    std::cout << "para1(" << a << "," << b << "," << c << "," << d << "," << e << ")\n";
}

int para01()
{
    std::cout << "para0()\n";
    return 0;
}

int para11(int a)
{
    std::cout << "para1(" << a << ")\n";
    return 0;
}

int para21(int a, int b)
{
    std::cout << "para1(" << a << "," << b << ")\n";
    return 0;
}


int para31(int a, int b, int c)
{
    std::cout << "para1(" << a << "," << b << "," << c << ")\n";
    return 0;
}


int para41(int a, int b, int c, int d)
{
    std::cout << "para1(" << a << "," << b << "," << c << "," << d << ")\n";
    return 0;
}


int para51(int a, int b, int c, int d, int e)
{
    std::cout << "para1(" << a << "," << b << "," << c << "," << d << "," << e << ")\n";
    return 0;
}


TEST(luausage, functional)
{
    cxx::lua::ScriptVM	vm(cxx::lua::ScriptVM::LIB_STRING |
                           cxx::lua::ScriptVM::LIB_DEBUG);


    try {
        // register class Test with constructors and member functions
        cxx::lua::module_(&vm)
            .class_<Test >("Test")
                .create(cxx::lua::constructor<TestA >())
                .create(cxx::lua::constructor<TestA, int >())
                .method("output",	&TestA::output)
                .method("add",		&TestA::add)
                .method("nothing",	&TestA::nothing)
                .method("nothing2",	&TestA::nothing2)
        ;

        // register a struct with member variables
        cxx::lua::module_(&vm)
            .class_<TestS >("TestS")
                .create(cxx::lua::constructor<TestS >())
                .member("a",	&TestS::a)
                .member("b",	&TestS::b);

        // register global functions
        cxx::lua::module_(&vm)
            .function("test1",	&test1)
            .function("test2",	&test2)
            .function("test_struct",	&test_struct)
            .function("test3",	&test3)
            .function("test4",	&test1);

        // register global functions
        cxx::lua::module_(&vm)
            .function("para0",	&para0)
            .function("para1", 	&para1)
            .function("para2",	&para2)
            .function("para3",	&para3)
            .function("para4",	&para4)
            .function("para5",	&para5)
            .function("para01",	&para01)
            .function("para11", &para11)
            .function("para21",	&para21)
            .function("para31",	&para31)
            .function("para41",	&para41)
            .function("para51",	&para51);

        cxx::lua::module_(&vm)
            .class_<TestVB >("TestVB")
            .method("create",		&TestVB::create)
            .method("do_something", &TestVB::do_something);

        cxx::lua::module_(&vm)
            .function("make_TestVB", &make_TestVB);

        {
            std::cout << "test case 101: virtual member function\n";
            vm.interpret("function abc(s)\n"
                         "  local x = s:do_something()\n"
                         "	print(x)\n"
                         "  return x\n"
                         "end\n"
                         "function xyz()\n"
                         "  local x = make_TestVB()\n"
                         "  return x\n"
                         "end\n", -1);
            TestVB* vb = new TestVI();
            int x = cxx::lua::script<int >::call(&vm, "abc", vb);
            assert(x == 100);
            if(x != 100) {
                std::cout << "test case 101: virtual member function failed\n";
            }
            delete vb;

            TestVB* vb2 = cxx::lua::script<TestVB* >::call(&vm, "xyz");
            delete vb2;
            std::cout << "test case 101 end\n\n";
        }

        {
            std::cout << "test case 001: interrept script\n";
            vm.interpret("print('12')\n", -1);
            std::cout << "test case 001 end\n\n";
        }

        {
            std::cout << "test case 002: interrept exported function\n";
            vm.interpret("test1()\n", -1);
            std::cout << "test case 002 end\n\n";
        }

        {
            std::cout << "test case 003: functions with same signture\n";
            vm.interpret("test1()\n"
                         "test2()\n", -1);
            std::cout << "test case 003 end\n\n";
        }
        {
            std::cout << "test case 004: register function alias\n";
            vm.interpret("test1()\n"
                         "test4()\n", -1);
            std::cout << "test case 004 end\n\n";
        }
        {
            std::cout << "test case 005: struct as parameter\n";
            vm.interpret("a = TestS()\n"
                         "a.a = 0\n"
                         "a.b = 0\n"
                         "test_struct(a)\n"
                         "a = nil\n", -1);
            std::cout << "test case 005 end\n\n";
        }
        {
            std::cout << "test case 006: using struct in script\n";
            vm.interpret("b = TestS()"
                         "b.a = 1\n"
                         "b.b = 1\n"
                         "print(b.a .. ' ' .. b.b)\n"
                         "b = nil", -1);
            std::cout << "test case 006 end\n\n";
        }
        {
            std::cout << "test case 007: up to 5 parameters\n";
            cxx::lua::script<void>::call(&vm, "para0");
            cxx::lua::script<void>::call(&vm, "para1", 1);
            cxx::lua::script<void>::call(&vm, "para2", 1, 2);
            cxx::lua::script<void>::call(&vm, "para3", 1, 2, 3);
            cxx::lua::script<void>::call(&vm, "para4", 1, 2, 3, 4);
            cxx::lua::script<void>::call(&vm, "para5", 1, 2, 3, 4, 5);
            cxx::lua::script<int >::call(&vm, "para01");
            cxx::lua::script<int >::call(&vm, "para11", 1);
            cxx::lua::script<int >::call(&vm, "para21", 1, 2);
            cxx::lua::script<int >::call(&vm, "para31", 1, 2, 3);
            cxx::lua::script<int >::call(&vm, "para41", 1, 2, 3, 4);
            cxx::lua::script<int >::call(&vm, "para51", 1, 2, 3, 4, 5);
            std::cout << "test case 007 end\n\n";
        }
        {
            std::cout << "test case 008: a complex script\n";
            TestS	ts;
            ts.a = 0;
            ts.b = 1;
            vm.interpret("function ppp(s)\n"
                         "	print(s.a ..\" \" .. s.b)\n"
                         "	test_struct(s)\n"
                         "	return s\n"
                         "end\n", -1);
            TestS x = cxx::lua::script<TestS >::call(&vm, "ppp", ts);
            std::cout << "  a=" << x.a << " b=" << x.b << "\n";
            std::cout << "test case 008 end\n\n";
        }
        {
            std::cout << "test case 009: return a struct\n";
            vm.interpret("function qqq()\n"
                         "	m = TestS()\n"
                         "	m.a = 3\n"
                         "	m.b = 3\n"
                         "	return m\n"
                         "end\n", -1);
            TestS x = cxx::lua::script<TestS >::call(&vm, "qqq");
            std::cout << "  a=" << x.a << " b=" << x.b << "\n";
            std::cout << "test case 009 end\n\n";
        }
        {
            std::cout << "test case 010: destructor\n";
            vm.interpret("d = Test()\n"
                         "d = nil\n", -1);
            cxx::lua::ScriptGC* gc = vm.collector();
            gc->collect();
            gc->destroy();
            std::cout << "test case 010 end\n\n";
        }
        {
            std::cout << "test case 011: call an exported function\n";
            cxx::lua::script<int >::call(&vm, "test1");
            cxx::lua::script<int >::call(&vm, "test2");
            cxx::lua::script<void >::call(&vm, "test3", 88);
            std::cout << "test case 011 end\n\n";
        }
        {
            std::cout << "test case 102: get a global from script\n";
            vm.interpret("aaa = 1\n"
                         "m = {x=1, y=2, z=3}"
                         "g = {\n"
                         " x= { p=1,q=2},y=0"
                         "}\n", -1);
            int a = cxx::lua::global<int >(&vm, "aaa");
            int b = cxx::lua::global<int >(&vm, "bbb");
            std::string c = cxx::lua::global<std::string >(&vm, "xxx");
            bool x = cxx::lua::isnull(&vm, "bbb");
            std::cout << "aaa=" << a << ",bbb=" << b << ",ccc=" << c << "\n";
            std::cout << "bbb is " << (x ? "not " : "") << "exist\n";

            cxx::lua::table t = cxx::lua::global<cxx::lua::table >(&vm, "m");
            int y = t.get<int >("y");
            int z = t.get<int >("z");
            std::cout << "m.y=" << y << ",m.z=" << z << "\n";

            cxx::lua::table t0 = cxx::lua::global<cxx::lua::table >(&vm, "g");
            cxx::lua::table t1 = t0.get<cxx::lua::table >("x");
            int p = t1.get<int >("p");
            int q = t1.get<int >("q");
            std::cout << "g.x.p=" << p << ",g.x.q=" << q << "\n";
            std::cout << "test case 102 end\n\n";
        }
        {
            std::cout << "test case 103: get a global vector from script\n";
            vm.interpret("a = {4,3,2,1}\n", -1);
            std::vector<int > a = cxx::lua::vector<int >(&vm, "a");
            assert(a.size() == 4);
            assert(a[0] == 4);
            assert(a[1] == 3);
            assert(a[2] == 2);
            assert(a[3] == 1);
            std::cout << "test case 103 end\n\n";
        }

        {
            std::cout << "test failed\n";
            try {
                vm.interpret("Test.output(\"abc\")", -1);
            }
            catch(const cxx::lua::ScriptError& e) {
                std::cout << "test failed 1 ok, " << e.what() << "\n";
            }
            try {
                vm.interpret("Test:mmm\n", -1);
            }
            catch(const cxx::lua::ScriptError& e) {
                std::cout << "test failed 2 ok, " << e.what() << "\n";
            }
            try {
                cxx::lua::script<int >::call(&vm, "notexist");
            }
            catch(const cxx::lua::ScriptError& e) {
                std::cout << "test failed 3 ok, " << e.what() << "\n";
            }
            try {
                vm.interpret("function qqq1()\n"
                             "	m = TestS()\n"
                             "	m.a = 3\n"
                             "	m.b = 3\n"
                             "  m:c = 8\n"
                             "	return m\n"
                             "end\n", -1);
                TestS x = cxx::lua::script<TestS >::call(&vm, "qqq1");
            }
            catch(const cxx::lua::ScriptError& e ) {
                std::cout << "test failed 4 ok, " << e.what() << "\n";
            }

            /// FIXME: this test case is passed with lua, but failed
            /// with luajit (segment fault).
            try {
                vm.interpret("a='abcdefg'\n"
                             "  string:find(a, \"x\")\n"
                             , -1);
            }
            catch(const cxx::lua::ScriptError& e ) {
                std::cout << "test failed 5 ok, " << e.what() << "\n";
            }
        }

    }
    catch(const cxx::lua::ScriptError& e) {
        std::cout << "!!!ERROR!!!\n"
            << e.what() << "\n";
    }
    catch(const std::runtime_error& e) {
        std::cerr << "runtime error: " << e.what() << "\n";
    }
    catch(...) {
        std::cerr << "unknown error\n";
    }
}



int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
