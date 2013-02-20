/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/alg/inputbuf.h"
#include <vector>
#include "common/sys/threads.h"
#include "common/sys/cputimes.h"

int test_count = 100;

TEST(ns_input_buffer, performance)
{
    cxx::alg::ns_input_buffer  buffer((void* )0, 1024*1024, 1);
    cxx::alg::ns_input_buffer::reader r = buffer.create_reader();
    cxx::alg::ns_input_buffer::writer w = buffer.create_writer();

    srand(time(NULL));
    for(size_t i = 0; i < test_count; ++i) {
        size_t size = random() % 512 + 1;
        void* p = w.reserve(size);
        *(size_t* )p = size;
        w.flush();

        void* q = NULL;
        size_t len = r.read(&q);
        ASSERT_EQ(len, size);
        ASSERT_EQ(*(size_t *)q, size);
        r.done();
    }
}

TEST(ts_input_buffer, performance)
{
    cxx::alg::ts_input_buffer  buffer(NULL, 1024*1024, 1);
    cxx::alg::ts_input_buffer::reader r = buffer.create_reader();
    cxx::alg::ts_input_buffer::writer w = buffer.create_writer();

    srand(time(NULL));
    for(size_t i = 0; i < test_count; ++i) {
        size_t size = random() % 512 + 1;
        void* p = w.reserve(size);
        *(size_t* )p = size;
        w.flush();

        void* q = NULL;
        size_t len = r.read(&q);
        ASSERT_EQ(len, size);
        ASSERT_EQ(*(size_t *)q, size);
        r.done();
    }
}

TEST(ps_input_buffer, performance)
{
    cxx::alg::ps_input_buffer  buffer(NULL, 1024*1024, 1, "test");
    cxx::alg::ps_input_buffer::reader r = buffer.create_reader();
    cxx::alg::ps_input_buffer::writer w = buffer.create_writer();

    srand(time(NULL));
    for(size_t i = 0; i < test_count; ++i) {
        size_t size = random() % 512 + 1;
        void* p = w.reserve(size);
        *(size_t* )p = size;
        w.flush();

        void* q = NULL;
        size_t len = r.read(&q);
        ASSERT_EQ(len, size);
        ASSERT_EQ(*(size_t *)q, size);
        r.done();
    }
}

class producer {
public:
    producer(std::vector<std::pair<int, int > >& expected, cxx::alg::input_buffer* buf)
        : exp_(expected), buf_(buf) {}
    void write() {
        cxx::alg::input_buffer::writer w(buf_);

        for(int i = 0; i < test_count; ++i) {
            const std::pair<int, int >& a = exp_[i % exp_.size()];
            void* p = NULL;
            int   s = a.first;
            if(s < sizeof(int)) {
                s = sizeof(int);
            }
            while(!p) {
                p = w.reserve(s);
                if(!p) {
                    cxx::sys::threadcontrol::sleep(1);
                }
            }
            *(int* )p = a.second;
            //printf("W: %d %d %d\n", i, s, a.second);
            w.flush();
        }
    }
private:
    std::vector<std::pair<int, int > >  exp_;
    cxx::alg::input_buffer*             buf_;
};

class consumer {
public:
    consumer(std::vector<std::pair<int, int > >& expected, cxx::alg::input_buffer* buf)
        : exp_(expected), buf_(buf) {}
    void read() {
        cxx::alg::input_buffer::reader r(buf_);

        try {
            for(int i = 0; i < test_count; ++i) {
                const std::pair<int, int >& a = exp_[i % exp_.size()];
                void* p = NULL;
                int   m = 0;
                int   s = a.first;
                if(s < sizeof(int)) {
                    s = sizeof(int);
                }
                while(!p) {
                    m = r.read(&p);
                    if(!p) {
                        cxx::sys::threadcontrol::sleep(1);
                    }
                }
                //printf("R: %d %d %d %d %d %s\n", i, s, a.second, m, *(int* )p,
                //       (s == m && a.second == *(int* )p) ? "OK" : "FAILED");
                ASSERT_EQ(s, m);
                ASSERT_EQ(a.second, *(int* )p);
                r.done();
            }
        }
        catch(const std::logic_error& e) {
            printf("std::logic_error: %s\n", e.what());
        }
    }
private:
    std::vector<std::pair<int, int > >  exp_;
    cxx::alg::input_buffer*             buf_;
};

TEST(ns_input_buffer, scsp)
{
    srand(time(NULL));
    std::vector<std::pair<int, int > > expected;
    for(int i = 0; i < test_count / 9; ++i) {
        int a = random() % 512;
        int b = random();
        expected.push_back(std::make_pair(a, b));
    }

    cxx::alg::ns_input_buffer  buf(NULL, 1024*1024, 1);
    producer                p(expected, &buf);
    consumer                c(expected, &buf);
    cxx::sys::thread tp = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&p, &producer::write), "procuder");
    cxx::sys::thread tc = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&c, &consumer::read), "consumer");
    tp.join();
    tc.join();
}

TEST(ns_input_buffer, scsp_int)
{
    srand(time(NULL));
    std::vector<std::pair<int, int > > expected;
    for(int i = 0; i < test_count / 9; ++i) {
        int a = 4;
        int b = i;
        expected.push_back(std::make_pair(a, b));
    }

    cxx::alg::ns_input_buffer  buf(NULL, 1024*1024, 1);

    producer                p(expected, &buf);
    consumer                c(expected, &buf);

    cxx::sys::cpu_times timer;
    cxx::sys::thread tc = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&c, &consumer::read), "consumer");
    cxx::sys::thread tp = cxx::sys::threadcontrol::create(cxx::MakeDelegate(&p, &producer::write), "procuder");
    tp.join();
    tc.join();

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

    test_count = 10*1000*1000;

    return RUN_ALL_TESTS();
}
