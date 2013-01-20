/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/sys/filesystem.h"
#include "common/sys/threads.h"
#include "common/gtest/gtest.h"
#include <stdio.h>

void infinity_loop()
{
    cxx::sys::threadcontrol::sleep(100);
    printf("thread started\n");
    int i = 0;
    while(true) {
        ++i;
    }
}

struct TestThread {
    TestThread(int i) : inst_(i) {}
    void run() {
        printf("TestThread::run %d\n", inst_);
        for(;;) {
            cxx::sys::threadcontrol::sleep(0);
        }
    }
    int inst_;
};

TEST(Path, pwd)
{
    cxx::sys::Path path1(cxx::sys::Path::curpath());
    printf("current path is: %s\n", path1.name());

    cxx::sys::Path path2(cxx::sys::Path::curpath());
    path2.append("abc");
    printf("path2 is: %s\n", path2.name());

    cxx::sys::Path path3("abc/");
    printf("path3 is: %s\n", path3.name());

    cxx::sys::Path path6("../");
    printf("path6 is: %s\n", path6.name());
}

TEST(Path, normalize)
{
    cxx::sys::Path path4("/abc/./xyz/../a");
    ASSERT_STREQ(path4.name(), "/abc/a");

    cxx::sys::Path path5("/abc/");
    path5.append("abc");
    ASSERT_STREQ(path5.name(), "/abc/abc");
}

TEST(Path, list)
{
    cxx::sys::DirIterator dir1(cxx::sys::Path::curpath());
    while(dir1 != cxx::sys::DirIterator()) {
        printf("files: %s\n", dir1.name().c_str());
        ++dir1;
    }
}

int main(int argc, char* argv[])
{
    cxx::sys::thread t[4];
    for(int i = 0; i < 4; ++i) {
        TestThread test(i);
        cxx::sys::threadcontrol::create(cxx::MakeDelegate(&test, &TestThread::run));
    }
    cxx::sys::threadcontrol::sleep(1000);
//    for(int i = 0; i < 4; ++i) {
//        t[i].join();
//    }

//    for(int i = 0; i < 4; ++i) {
//        t[i] = cxx::sys::threadcontrol::create(&infinity_loop, "loop");
//    }
//    for(int i = 0; i < 4; ++i) {
//        t[i].join();
//    }

    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();

}

