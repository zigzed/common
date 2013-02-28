/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/sys/threads.h"
#include "common/sys/cputimes.h"
#include "common/alg/channel.h"
#include <list>

class PipeTester {
public:
    PipeTester(int count, int sleep)
        : c_(count), s_(sleep)
    {
    }

    void reader()
    {
        int count = 0;
        int sleep = 0;
        while(count < c_) {
            int x = 0;
            if(p_.recv(&x, sleep)) {
                if(x != count) {
                    std::cout << "reader failed: " << x << "," << count << "\n";
                }
                ++count;
            }
            else {
                sleep ++;
            }
        }
        std::cout << "reader sleep: " << sleep
                  << " channel size: " << p_.size()
                  << "\n";
    }

    void writer()
    {
        int count = 0;
        while(count < c_) {
            p_.send(count);
            ++count;
        }
    }

private:
    cxx::alg::channel<int, 256 >    p_;
    int                             c_;
    int                             s_;
};


TEST(channel, performance)
{
    PipeTester t(10*1000*1000, 100);
    cxx::sys::cpu_times timer;
    cxx::sys::thread t1 = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&t, &PipeTester::reader), "reader");
    cxx::sys::thread t2 = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&t, &PipeTester::writer), "writer");
    t1.join();
    t2.join();
    timer.stop();
    cxx::sys::cpu_times::times usage = timer.elapsed();

    double sec = 1000000000.0L;
    printf("wall: %f total: %f user: %f sys: %f usage: %f\%\n",
           usage.wall / sec, (usage.user + usage.sys) / sec,
           usage.user / sec, usage.sys / sec, ((usage.user + usage.sys) * 100.0 / usage.wall));
}


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

