/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/sys/event.h"
#include "common/sys/threads.h"

class EventTester {
public:
    EventTester(int count, int sleep)
        : count_(count), sleep_(sleep) {}

    void reader()
    {
        int readed = 0;
        while(readed != count_) {
            if(event_.wait(sleep_)) {
                event_.recv();
                ++readed;
            }
        }
        std::cout << "reader done\n";
    }
    void writer()
    {
        int writen = 0;
        while(writen != count_) {
            event_.send();
            ++writen;
            cxx::sys::threadcontrol::sleep(sleep_);
        }
        std::cout << "writer done\n";
    }
private:
    cxx::sys::event event_;
    int             count_;
    int             sleep_;
};

TEST(event, read_write)
{
    EventTester t(100, 100);
    cxx::sys::thread t1 = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&t, &EventTester::reader), "reader");
    cxx::sys::thread t2 = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&t, &EventTester::writer), "writer");
    t1.join();
    t2.join();
}

TEST(event, write_read)
{
    EventTester t(100, 0);
    cxx::sys::thread t2 = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&t, &EventTester::writer), "writer");
    cxx::sys::threadcontrol::sleep(1000);
    cxx::sys::thread t1 = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&t, &EventTester::reader), "reader");
    t2.join();
    t1.join();
}



int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
