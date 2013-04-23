/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/alg/url.h"
#include "common/sys/cputimes.h"

TEST(URL, simple)
{
    std::string testurl = "$abcAX*?# M._+=";
    std::string encoded = cxx::alg::url::encode(testurl);
    std::string decoded = cxx::alg::url::decode(encoded);
    ASSERT_EQ(decoded, testurl);
    printf("encoded url: %s\n", encoded.c_str());
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
