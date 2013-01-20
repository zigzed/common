/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include <fstream>
#include <stdlib.h>
#include "common/gtest/gtest.h"
#include "common/sys/threads.h"
#include "common/str/tokenizer.h"
#ifdef OS_WINDOWS
#include<tlhelp32.h>
#endif

static bool running = true;

void test_thread()
{
    while(running) {
        cxx::sys::threadcontrol::sleep(1);
    }
}

int thread_count()
{
#ifdef OS_LINUX
    std::ifstream ifs("/proc/self/stat", std::ios::in);
    std::string line;
    if(std::getline(ifs, line)) {
        cxx::str::tokenizer token(line, " ");
        if(token.size() >= 23) {
            return atol(token[19].c_str());
        }
    }
    return 0;
#else
	DWORD hPid = GetCurrentProcessId();
	HANDLE hThreadSnap = NULL;
	THREADENTRY32 te32 = {0};

	// 拍摄一张含有系统所有线程的快照
	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
	if(hThreadSnap == (HANDLE)(-1)) return 0;
	
	te32.dwSize = sizeof(THREADENTRY32);
	if(Thread32First(hThreadSnap,&te32))
	{
		int count = 0;
		do
		{
			if(te32.th32OwnerProcessID == hPid)
				count ++;

		}
		while(Thread32Next(hThreadSnap, &te32));
		return count;
	}
	return 0;

#endif
}

struct TestThread {
    void init() {
        r = true;
        t = cxx::sys::threadcontrol::create(cxx::MakeDelegate(this, &TestThread::run));
    }

    void run() {
        while(r) {
            cxx::sys::threadcontrol::sleep(1);
        }
    }
    void stop() {
        r = false;
        t.join();
    }

    cxx::sys::thread t;
    bool             r;
};

TEST(threads, count)
{
    cxx::sys::thread t[100];
    for(int i = 0; i < 100; ++i) {
        t[i] = cxx::sys::threadcontrol::create(&test_thread, "test_thread");
    }
    ASSERT_EQ(101, thread_count());

    running = false;

    for(int i = 0; i < 100; ++i) {
        t[i].join();
    }
}

TEST(threads, count2)
{
    TestThread t[100];
    for(int i = 0; i < 100; ++i) {
        t[i].init();
    }
    ASSERT_EQ(101, thread_count());

    for(int i = 0; i < 100; ++i) {
        t[i].stop();
    }
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}


