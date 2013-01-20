/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/alg/lrucache.h"
#include "common/gtest/gtest.h"
#include <string>
#include <iostream>

TEST(lrucache, simple)
{
    cxx::alg::LruCache<int, int > cache(3);

    cache.input(1, 1);
    cache.input(2, 2);
    cache.input(3, 3);
    ASSERT_EQ(cache.size(), 3);

    cache.input(4, 4);
    ASSERT_EQ(cache.size(), 3);

    ASSERT_EQ(cache.fetch(1, false), (int* )0);
    ASSERT_EQ(*cache.fetch(2, false), 2);
    ASSERT_EQ(*cache.fetch(3, false), 3);
    ASSERT_EQ(*cache.fetch(4, false), 4);

    printf("lrucache::simple, hit ratio: %5.2f\%\n", cache.ratio()*100);
}

struct SizeSizer {
    std::size_t operator()(int k, int v) const {
        return sizeof(k) + sizeof(v);
    }
};

template<typename K, typename D >
struct CoutSaver {
    void operator()(const K& k, const D& v) const {
        std::cout << k << ":" << v << "\n";
    }
};

TEST(lrucache, complex)
{
    cxx::alg::LruCache<int, int, SizeSizer, CoutSaver<int, int > > cache(32);

    cache.input(1, 1);
    cache.input(2, 2);
    cache.touch(1);
    cache.input(3, 3);
    cache.touch(1);
    cache.input(4, 4);
    cache.input(5, 5);
    cache.input(6, 6);
    // 预期结果：屏幕输出 2:2, 3:3
    printf("lrucache::complex, hit ratio: %5.2f\%\n", cache.ratio()*100);
}

struct StringSizer {
    std::size_t operator()(const std::string& k, int v) const {
        return k.size() + sizeof(v);
    }
};

TEST(lrucache, string)
{
    int size = 100000;
    cxx::alg::LruCache<std::string, int, StringSizer, CoutSaver<std::string, int > > cache(size);
    char key[256];
    for(int i = 0; i < size / 10; ++i) {
        sprintf(key, "key-%d", i);
        cache.input(key, i);
    }
    // 预期结果：屏幕输出 key-0:0,...,key-1666:1666

    for(int i = size / 10 - 2000; i < size / 10; ++i) {
        sprintf(key, "key-%d", i);
        int* tmp = cache.fetch(std::string(key), false);
        ASSERT_EQ(*tmp, i);
    }

    printf("lrucache::string, hit ratio: %5.2f\%\n", cache.ratio()*100);
}

struct TestLoader {
    bool operator()(int k, int& v) {
        if(k >= 100 && k < 200) {
            v = k*2;
            return true;
        }
        return false;
    }
};

TEST(lrucache, loader)
{
    int size = 1000;
    cxx::alg::LruCache<int, int,
            cxx::alg::CountSizer<int, int >,
            cxx::alg::NullKeeper<int, int >,
            TestLoader > cache(1000);

    for(int i = 0; i < 100; ++i) {
        cache.input(i, i);
    }

    for(int i = 0; i < 100; ++i) {
        ASSERT_EQ(*cache.fetch(i, false), i);
    }

    // 调用 TestLoader 从外部加载数据
    for(int i = 100; i < 200; ++i) {
        ASSERT_EQ(*cache.fetch(i, false), i * 2);
    }

    // 直接从缓存获取数据
    for(int i = 100; i < 200; ++i) {
        ASSERT_EQ(*cache.fetch(i, false), i * 2);
    }

    for(int i = 200; i < 300; ++i) {
        ASSERT_EQ(cache.fetch(i, false), (int* )0);
    }

    printf("lrucache::loader, hit ratio: %5.2f\%\n", cache.ratio()*100);
}

TEST(lrucache, ratio)
{
    cxx::alg::LruCache<int, int > cache(10);
    for(int i = 0; i < 10; ++i) {
        cache.input(i, i);
    }
    for(int i = 0; i < 20; ++i) {
        cache.fetch(i, false);
    }
    ASSERT_FLOAT_EQ(cache.ratio(), 0.5);

    for(int i = 0; i < 10; ++i) {
        cache.fetch(i, false);
    }
    ASSERT_FLOAT_EQ(cache.ratio(), (float)20/30);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
