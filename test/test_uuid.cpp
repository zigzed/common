/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/sys/uuid.h"
#include <sstream>
#include <algorithm>

TEST(uuid, generator)
{
    cxx::sys::uuid u1 = cxx::sys::nil_uuid()();
    printf("u1=%s\n", cxx::sys::to_string(u1).c_str());
    ASSERT_EQ(u1.is_null(), true);

    cxx::sys::uuid u2 = cxx::sys::sys_uuid()();
    printf("u2=%s\n", cxx::sys::to_string(u2).c_str());
    ASSERT_EQ(u2.is_null(), false);


    ASSERT_NE(u1, u2);
    ASSERT_LT(u1, u2);

    std::size_t h1 = u1.hash();
    std::size_t h2 = u2.hash();
    ASSERT_NE(h1, h2);
}

TEST(uuid, io)
{
    cxx::sys::uuid u = cxx::sys::sys_uuid()();
    std::string s = cxx::sys::to_string(u);
    cxx::sys::uuid f;
    std::istringstream is(s);
    is >> f;
    std::string t = cxx::sys::to_string(f);

    printf("%s\n%s\n", s.c_str(), t.c_str());

    ASSERT_EQ(f, u);
    ASSERT_EQ(s, t);
}

TEST(uuid, duplicated)
{
    int count = 1000000;
    std::vector<cxx::sys::uuid >  us;
    us.reserve(count);
    for(int i = 0; i < count; ++i) {
        us.push_back(cxx::sys::sys_uuid()());
    }
    std::sort(us.begin(), us.end());
    std::vector<cxx::sys::uuid >::iterator it = std::unique(us.begin(), us.end());
    ASSERT_EQ(std::distance(us.begin(), it), count);

    printf("%s\n%s\n", cxx::sys::to_string(us.front()).c_str(),
           cxx::sys::to_string(us.back()).c_str());
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
