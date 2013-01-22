/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include <limits.h>
#include <climits>
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/alg/rlcbitmap.h"
#include "common/sys/cputimes.h"

template<typename uword >
void test_set_get()
{
    cxx::alg::rlc_bitmap<uword >   bitmap;
    uint32_t val[] = {5, 4400, 44600, 55400, 1000000 };
    for(int k = 0; k < sizeof(val)/sizeof(val[0]); ++k) {
        bitmap.set(val[k]);
    }

    std::cout << "used memory: " << bitmap.memory() << "bytes, 1s: " << bitmap.count()
              << ", bits: " << bitmap.bitlen() << "\n";

    size_t counter = 0;
    for(typename cxx::alg::rlc_bitmap<uword >::const_iterator it = bitmap.begin();
        it != bitmap.end(); ++it) {
        ASSERT_EQ(*it, val[counter++]);
    }

    typename cxx::alg::rlc_bitmap<uword >::bit_array array;
    array = bitmap.bitset();
    for(size_t i = 0; i < array.size(); ++i) {
        ASSERT_EQ(array[i], val[i]);
    }
}

template<typename uword >
void test_RLW()
{
    uword some_number(0xABCD);
    cxx::alg::detail::RunLengthWord<uword > rlw(some_number);
    rlw.set_running_bit(true);
    ASSERT_EQ(rlw.get_running_bit(), true);

    for(uword rl = 0; rl <= cxx::alg::detail::RunLengthWord<uword >::LargestRunningLengthCount;
        rl = static_cast<uword >(rl + cxx::alg::detail::RunLengthWord<uword >::LargestRunningLengthCount / 10)) {
        rlw.set_running_len(rl);
        ASSERT_EQ(rlw.get_running_bit(), true);
        ASSERT_EQ(rlw.get_running_len(), rl);
    }

    rlw.set_running_len(12);
    for(uword lw = 0; lw <= cxx::alg::detail::RunLengthWord<uword >::LargestLiteralCount;
        lw = static_cast<uword >(lw + cxx::alg::detail::RunLengthWord<uword >::LargestLiteralCount / 10)) {
        rlw.set_literal_len(lw);
        ASSERT_EQ(rlw.get_running_bit(), true);
        ASSERT_EQ(rlw.get_running_len(), 12);
        ASSERT_EQ(rlw.get_literal_len(), lw);
    }

    rlw.set_literal_len(43);
    rlw.set_running_bit(false);
    ASSERT_EQ(rlw.get_running_bit(), false);
    ASSERT_EQ(rlw.get_running_len(), 12);
    ASSERT_EQ(rlw.get_literal_len(), 43);
}

template<typename T >
struct limit {
    static const T max;
};

template<>
struct limit<uint16_t > {
    static const uint16_t max = USHRT_MAX;
};

template<>
struct limit<uint32_t > {
    static const uint32_t max = UINT_MAX;
};

template<>
struct limit<uint64_t > {
    static const uint64_t max = ULONG_MAX;
};


template<typename uword >
void test_bitset()
{
    srand(time(NULL));
    cxx::sys::cpu_times timer;
    timer.stop();
    for(int i = 0; i < 1000; ++i) {
        cxx::alg::rlc_bitmap<uword >   bitmap;
        int     cnt = random() % 400;
        uint32_t*  val = new uint32_t[cnt];
        for(int j = 0; j < cnt; ++j) {
            val[j] = ((unsigned long)random()) % 1700000;
        }
        std::sort(val, val + cnt);
        uint32_t*  tmp = std::unique(val, val + cnt);
        cnt = tmp - val;
        for(int j = 0; j < cnt; ++j) {
            bitmap.set(val[j]);
        }

        timer.resume();
        typename cxx::alg::rlc_bitmap<uword >::bit_array array = bitmap.bitset();
        timer.stop();

        ASSERT_EQ(array.size(), cnt);
        for(int j = 0; j < cnt; ++j) {
            ASSERT_EQ(array[j], val[j]);
        }

        if(i % 100 == 0) {
            std::cout << i << ": " << "elements: " << cnt << ", "
                      << "used memory: " << bitmap.memory() << "bytes, 1s: " << bitmap.count()
                      << ", bits: " << bitmap.bitlen() << "\n";
            cxx::alg::bitmap_stat stat = bitmap.stat();
            std::cout << i << ":" << "compressed: " << stat.get_compressed() << ", "
                      << "uncompressed: " << stat.get_uncompressed() << ", "
                      << "over run: " << stat.get_over_runs() << "\n";
        }

        delete[] val;
    }
    std::cout << "used timer: " << timer.elapsed().wall / 1000 << "us\n";
}

template<typename uword >
void test_and()
{
    {
        cxx::alg::rlc_bitmap<uword > test, test1, test2, test3;
        for(uint32_t i = 0; i <= 10000; ++i) {
            test.set(i);
            // because test1 is empty, the result shoud be empty too
            test._and(test1, test2);
            test3 = test & test1;

            typename cxx::alg::rlc_bitmap<uword >::bit_array result;
            result = test2.bitset();
            ASSERT_EQ(result.size(), 0);
            result = test3.bitset();
            ASSERT_EQ(result.size(), 0);

            ASSERT_EQ(test2.count(), 0);
            ASSERT_EQ(test3.count(), 0);

            ASSERT_EQ(test2 == test3, true);
        }
    }
    {
        cxx::alg::rlc_bitmap<uword > test, test1, test2, test3;
        test1.set(1);
        test1.set(3);
        test1.set(5);
        test2.set(2);
        test2.set(4);
        test2.set(6);
        test = test1 & test2;

        typename cxx::alg::rlc_bitmap<uword >::bit_array result;
        result = test.bitset();
        ASSERT_EQ(result.size(), 0);

        test.reset();
        test = test1 | test2;
        result = test.bitset();
        ASSERT_EQ(result.size(), 6);

        for(int i = 1; i < 7; ++i) {
            ASSERT_EQ(result[i - 1], i);
        }
    }
    {
        cxx::alg::rlc_bitmap<uword > test, test1, test2, test3;
        test1.set(1);
        test1.set(3);
        test1.set(5);
        test2.set(2);
        test2.set(4);
        test = test1 & test2;

        typename cxx::alg::rlc_bitmap<uword >::bit_array result;
        result = test.bitset();
        ASSERT_EQ(result.size(), 0);

        test.reset();
        test = test1 | test2;
        result = test.bitset();
        ASSERT_EQ(result.size(), 5);

        for(int i = 1; i < 6; ++i) {
            ASSERT_EQ(result[i - 1], i);
        }
    }
    {
        cxx::alg::rlc_bitmap<uword > test, test1, test2, test3;
        test1.set(1);
        test1.set(3);
        test2.set(2);
        test2.set(4);
        test2.set(5);
        test = test1 & test2;

        typename cxx::alg::rlc_bitmap<uword >::bit_array result;
        result = test.bitset();
        ASSERT_EQ(result.size(), 0);

        test.reset();
        test = test1 | test2;
        result = test.bitset();
        ASSERT_EQ(result.size(), 5);

        for(int i = 1; i < 6; ++i) {
            ASSERT_EQ(result[i - 1], i);
        }
    }
    {
        cxx::alg::rlc_bitmap<uword > test, test1, test2, test3;
        test1.set(1);
        test1.set(2);
        test1.set(5);
        test2.set(2);
        test2.set(4);
        test2.set(6);
        test2.set(1000);
        test = test1 & test2;

        typename cxx::alg::rlc_bitmap<uword >::bit_array result;
        result = test.bitset();
        ASSERT_EQ(result.size(), 1);
        ASSERT_EQ(result[0], 2);

        test = test1 | test2;
        result = test.bitset();
        ASSERT_EQ(result.size(), 6);

        ASSERT_EQ(result[0], 1);
        ASSERT_EQ(result[1], 2);
        ASSERT_EQ(result[2], 4);
        ASSERT_EQ(result[3], 5);
        ASSERT_EQ(result[4], 6);
        ASSERT_EQ(result[5], 1000);
    }
    {
        cxx::alg::rlc_bitmap<uword > test, test1, test2, test3;
        test1.set(1);
        test1.set(2);
        test1.set(5);
        test1.set(1000);
        test2.set(2);
        test2.set(4);
        test2.set(6);
        test = test1 & test2;

        typename cxx::alg::rlc_bitmap<uword >::bit_array result;
        result = test.bitset();
        ASSERT_EQ(result.size(), 1);
        ASSERT_EQ(result[0], 2);

        test = test1 | test2;
        result = test.bitset();
        ASSERT_EQ(result.size(), 6);

        ASSERT_EQ(result[0], 1);
        ASSERT_EQ(result[1], 2);
        ASSERT_EQ(result[2], 4);
        ASSERT_EQ(result[3], 5);
        ASSERT_EQ(result[4], 6);
        ASSERT_EQ(result[5], 1000);
    }
    {
        cxx::alg::rlc_bitmap<uword > test, test1, test2, test3;
        test1.set(1);
        test1.set(2);
        test1.set(5);
        test1.set(1000);
        test2.set(2);
        test2.set(4);
        test2.set(6);
        test2.set(1000);
        test = test1 & test2;

        typename cxx::alg::rlc_bitmap<uword >::bit_array result;
        result = test.bitset();
        ASSERT_EQ(result.size(), 2);
        ASSERT_EQ(result[0], 2);
        ASSERT_EQ(result[1], 1000);

        test = test1 | test2;
        result = test.bitset();
        ASSERT_EQ(result.size(), 6);

        ASSERT_EQ(result[0], 1);
        ASSERT_EQ(result[1], 2);
        ASSERT_EQ(result[2], 4);
        ASSERT_EQ(result[3], 5);
        ASSERT_EQ(result[4], 6);
        ASSERT_EQ(result[5], 1000);
    }
}

template<typename uword >
void test_not()
{
    cxx::alg::rlc_bitmap<uword > bitmap;
    for(int i = 0; i <= 184; ++i) {
        bitmap.set(i);
    }
    ASSERT_EQ(bitmap.count(), 185);

    cxx::alg::rlc_bitmap<uword > temp = ~bitmap;
    ASSERT_EQ(temp.count(), 7);

    bitmap._not();
    std::cout << "~bitmap.count()=" << bitmap.count() << "\n";
    ASSERT_EQ(bitmap.count(), 7);
}

template<typename uword >
void test_bitwise()
{
    cxx::alg::rlc_bitmap<uword >   array1, array2;
    static const uint32_t    elem = 15;
    uword       all1 = static_cast<uword >(~0L);
    uword       x1[elem] = {1, 54, 24, 145, 0, 0, 0, all1, all1, all1, 43, 0, 0, 0, 1};
    uword       x2[elem] = {all1, 0, 0, 0, 0, 0, 0, 0, all1, all1, all1, 0, 4, 0, 0};
    uword       xand[elem];
    uword       xxor[elem];

    for(uint32_t k = 0; k < elem; ++k) {
        array1.add(x1[k]);
        array2.add(x2[k]);
        xand[k] = x1[k] & x2[k];
        xxor[k] = x1[k] | x2[k];
    }

    cxx::alg::rlc_bitmap<uword >   bit_and;
    cxx::alg::rlc_bitmap<uword >   bit_or;

    bit_and = array1 & array2;
    bit_or  = array1 | array2;

    cxx::alg::rlc_bitmap_iterator<uword > i = bit_and.uncompress();
    cxx::alg::rlc_bitmap_iterator<uword > j = bit_or.uncompress();
    cxx::alg::rlc_bitmap_iterator<uword > it1 = array1.uncompress();
    cxx::alg::rlc_bitmap_iterator<uword > it2 = array2.uncompress();

    for(uint32_t k = 0; k < elem; ++k) {
        uword m1 = it1.next();
        uword m2 = it2.next();
        ASSERT_EQ(i.has_next(), true);
        ASSERT_EQ(j.has_next(), true);
        ASSERT_EQ(i.next(), xand[k]);
        ASSERT_EQ(j.next(), xxor[k]);
    }
}

template<typename uword >
void test_perf(bool is_and)
{
    cxx::sys::cpu_times timer;
    timer.stop();

    srand(time(NULL));
    for(int i = 0; i < 100000; ++i) {
        cxx::alg::rlc_bitmap<uword >   bitmap1;
        cxx::alg::rlc_bitmap<uword >   bitmap2;
        int         cnt = random() % 50;
        uint32_t*  val1 = new uint32_t[cnt];
        uint32_t*  val2 = new uint32_t[cnt];
        for(int j = 0; j < cnt; ++j) {
            val1[j] = ((unsigned long)random()) % limit<uint32_t >::max;
            val2[j] = ((unsigned long)random()) % limit<uint32_t >::max;
        }
        std::sort(val1, val1 + cnt);
        std::sort(val2, val2 + cnt);
        uint32_t*  tmp1 = std::unique(val1, val1 + cnt);
        uint32_t*  tmp2 = std::unique(val2, val2 + cnt);

        for(int j = 0; j < tmp1 - val1; ++j) {
            bitmap1.set(val1[j]);
        }
        for(int j = 0; j < tmp2 - val2; ++j) {
            bitmap2.set(val2[j]);
        }

        timer.resume();
        if(is_and) {
            cxx::alg::rlc_bitmap<uword > bitmap3 = bitmap1 & bitmap2;
        }
        else {
            cxx::alg::rlc_bitmap<uword > bitmap3 = bitmap1 | bitmap2;
        }
        timer.stop();

        delete[] val1;
        delete[] val2;
    }
    std::cout << "used timer: " << timer.elapsed().wall / 1000 << "us\n";
}

TEST(BITMAP, set_get)
{
    test_set_get<uint16_t >();
    test_set_get<uint32_t >();
    test_set_get<uint64_t >();
}

TEST(BITMAP, RLW)
{
    test_RLW<uint16_t >();
    test_RLW<uint32_t >();
    test_RLW<uint64_t >();
}

TEST(BITMAP, AND)
{
    test_and<uint16_t >();
    test_and<uint32_t >();
    test_and<uint64_t >();
}

TEST(BITMAP, NOT)
{
    test_not<uint16_t >();
    test_not<uint32_t >();
    test_not<uint64_t >();
}

TEST(BITMAP, BITWISE)
{
    test_bitwise<uint16_t >();
    test_bitwise<uint32_t >();
    test_bitwise<uint64_t >();
}

TEST(BITMAP, PERF_AND_32)
{
    test_perf<uint32_t >(true);
}

TEST(BITMAP, PERF_OR_32)
{
    test_perf<uint32_t >(false);
}


TEST(BITMAP, BITSET)
{
    test_bitset<uint16_t >();
    test_bitset<uint32_t >();
    test_bitset<uint64_t >();
}


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

