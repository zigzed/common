/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/alg/pipe.h"
#include "common/sys/threads.h"
#include "common/sys/event.h"
#include "common/sys/cputimes.h"
#include "common/sys/mutex.h"
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
            if(p_.read(&x)) {
                if(x != count) {
                    std::cout << "reader failed: " << x << "," << count << "\n";
                }
                ++count;
            }
            else {
                cxx::sys::threadcontrol::sleep(s_);
                sleep ++;
            }
        }
        std::cout << "reader sleep: " << sleep << "\n";
    }

    void writer()
    {
        int count = 0;
        int flush = 0;
        while(count < c_) {
            p_.write(count, false);
            if(!p_.flush()) {
                ++flush;
            }
            ++count;
        }
        p_.flush();
    }

private:
    cxx::alg::pipe<int, 256 >  p_;
    int             c_;
    int             s_;
};

class PipeTesterNotify {
public:
    PipeTesterNotify(int count, int sleep)
        : c_(count), s_(sleep), active(true)
    {
        p_.read(NULL);
    }

    void reader()
    {
        int count = 0;
        int sleep = 0;
        int a_cnt = 0;
        int p_cnt = 0;

        while(count < c_) {
            int x = 0;
            if(active) {
                bool ok = p_.read(&x);
                if(ok) {
                    if(x != count)
                        std::cout << "reader failed: " << x << "," << count << "\n";
                    ++a_cnt;
                    ++count;
                    continue;
                }
                active = false;
                e_.recv();
            }

            if(e_.wait(s_ * 1000)) {
                sleep++;
                active = true;
                bool ok = p_.read(&x);
                if(ok) {
                    if(x != count)
                        std::cout << "reader failed: " << x << "," << count << "\n";
                    ++p_cnt;
                    ++count;
                    continue;
                }
            }
        }
        std::cout << "reader sleep: " << sleep << ", a: " << a_cnt << ", p: "
                  << p_cnt << "\n";
    }

    void writer()
    {
        int count = 0;
        int flush = 0;
        while(count < c_) {
            p_.write(count, false);
            bool ok = p_.flush();
            if(!ok) {
                ++flush;
                e_.send();
            }
            ++count;
        }
    }

private:
    cxx::alg::pipe<int, 256 >  p_;
    cxx::sys::event e_;
    int             c_;
    int             s_;
    bool            active;
};

class PipeLockTester {
public:
    PipeLockTester(int count, int sleep)
        : count_(count), sleep_(sleep) {}
    void reader() {
        int count = 0;
        int sleep = 0;
        while(count < count_) {
            if(queue_.empty()) {
                cxx::sys::threadcontrol::sleep(sleep_);
                sleep ++;
                continue;
            }
            int x = 0;
            cxx::sys::plainmutex::scopelock lock(mutex_);
            if(!queue_.empty()) {
                x = queue_.front();
                if(x != count)
                    std::cout << "reader failed: " << x << "," << count << "\n";
                queue_.pop_front();
                count++;
            }
        }
        std::cout << "reader sleep: " << sleep << "\n";
    }
    void writer() {
        int count = 0;
        while(count < count_) {
            cxx::sys::plainmutex::scopelock lock(mutex_);
            queue_.push_back(count);
            ++count;
        }
    }

private:
    cxx::sys::plainmutex mutex_;
    std::list<int >      queue_;
    int                  count_;
    int                  sleep_;
};

class BatchPipeLockTester {
public:
    BatchPipeLockTester(int count, int sleep)
        : count_(count), sleep_(sleep) {}
    void reader() {
        int count = 0;
        int sleep = 0;
        while(count < count_) {
            std::list<int > temp;
            {
                cxx::sys::plainmutex::scopelock lock(mutex_);
                temp.swap(queue_);
            }
            if(temp.empty()) {
                cxx::sys::threadcontrol::sleep(sleep_);
                sleep ++;
                continue;
            }
            std::list<int >::const_iterator it = temp.begin();
            for(; it != temp.end(); ++it) {
                int x = *it;
                if(x != count)
                    std::cout << "reader failed: " << x << "," << count << "\n";
                count++;
            }
        }
        std::cout << "reader sleep: " << sleep << "\n";
    }
    void writer() {
        int count = 0;
        while(count < count_) {
            cxx::sys::plainmutex::scopelock lock(mutex_);
            queue_.push_back(count);
            ++count;
        }
    }

private:
    cxx::sys::plainmutex mutex_;
    std::list<int >      queue_;
    int                  count_;
    int                  sleep_;
};

TEST(pipe, performance)
{
    PipeTester t(10*1000*1000, 1);
    cxx::sys::cpu_times timer;
    cxx::sys::thread t2 = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&t, &PipeTester::writer), "writer");
    cxx::sys::thread t1 = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&t, &PipeTester::reader), "reader");
    t1.join();
    t2.join();
    timer.stop();
    cxx::sys::cpu_times::times usage = timer.elapsed();

    double sec = 1000000000.0L;
    printf("wall: %f total: %f user: %f sys: %f usage: %f\%\n",
           usage.wall / sec, (usage.user + usage.sys) / sec,
           usage.user / sec, usage.sys / sec, ((usage.user + usage.sys) * 100.0 / usage.wall));
}

TEST(pipe, performance2)
{
    PipeTesterNotify t(10*1000*1000, 1);
    cxx::sys::cpu_times timer;
    cxx::sys::thread t2 = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&t, &PipeTesterNotify::writer), "writer");
    cxx::sys::thread t1 = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&t, &PipeTesterNotify::reader), "reader");
    t1.join();
    t2.join();
    timer.stop();
    cxx::sys::cpu_times::times usage = timer.elapsed();

    double sec = 1000000000.0L;
    printf("wall: %f total: %f user: %f sys: %f usage: %f\%\n",
           usage.wall / sec, (usage.user + usage.sys) / sec,
           usage.user / sec, usage.sys / sec, ((usage.user + usage.sys) * 100.0 / usage.wall));

}

TEST(lock_pipe, performance)
{
    PipeLockTester t(10*1000*1000, 1);
    cxx::sys::cpu_times timer;
    cxx::sys::thread t2 = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&t, &PipeLockTester::writer), "writer");
    cxx::sys::thread t1 = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&t, &PipeLockTester::reader), "reader");
    t1.join();
    t2.join();
    timer.stop();
    cxx::sys::cpu_times::times usage = timer.elapsed();

    double sec = 1000000000.0L;
    printf("wall: %f total: %f user: %f sys: %f usage: %f\%\n",
           usage.wall / sec, (usage.user + usage.sys) / sec,
           usage.user / sec, usage.sys / sec, ((usage.user + usage.sys) * 100.0 / usage.wall));
}

TEST(batch_lock_pipe, performance)
{
    BatchPipeLockTester t(10*1000*1000, 1);
    cxx::sys::cpu_times timer;
    cxx::sys::thread t2 = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&t, &BatchPipeLockTester::writer), "writer");
    cxx::sys::thread t1 = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&t, &BatchPipeLockTester::reader), "reader");
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

