#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/str/regex.h"

TEST(Regexp, isdigit)
{
    cxx::str::regexp    regex("\\d+");
    ASSERT_EQ(regex.MatchExact("123456").IsMatched(), true);
    ASSERT_EQ(regex.MatchExact("12345A").IsMatched(), false);
}

TEST(Regexp, email_address)
{
    cxx::str::regexp    regex("^([0-9a-zA-Z]([-.\\w]*[0-9a-zA-Z])*@(([0-9a-zA-Z])+([-\\w]*[0-9a-zA-Z])*\\.)+[a-zA-Z]{2,9})$");
    ASSERT_EQ(regex.MatchExact("abc@example.com").IsMatched(), true);
    ASSERT_EQ(regex.MatchExact("abc@.com").IsMatched(), false);
}

TEST(Regexp, loop)
{
    cxx::str::regexp    regex("\\b\\d+\\.\\d+");
    const char* example = "12.5,a11,0.123,178";
    const char* expects[] = { "12.5", "0.123" };

    cxx::str::MatchResult result = regex.Match(example);
    int matched = 0;
    while(result.IsMatched()) {
        ASSERT_STREQ(std::string(example + result.GetStart(), example + result.GetEnd()).c_str(), expects[matched++]);

        printf("%.*s\n", result.GetEnd() - result.GetStart(), example + result.GetStart());
        result = regex.Match(example, result.GetEnd());
    }
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
