#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/sql/sqlite++.h"
#include "common/sys/filesystem.h"
#include <stdio.h>

TEST(sqlite, open)
{
    ::remove("abc.db");

    cxx::sqlite3_connection sql("abc.db");
    sql.executedml("create table test (a INT, b REAL, c TEXT)");

    sql.executedml("insert into test values(1, 1.1, \"abc\")");

    cxx::sqlite3_command    cmd(sql, "select a, b, c from test");
    cxx::sqlite3_reader     rdr = cmd.execute((cxx::sqlite3_reader* )NULL);
    while(!rdr.iseof()) {
        int         a = rdr.get(0, (int* )NULL);
        double      b = rdr.get(1, (double* )NULL);
        std::string c = rdr.get(2, (std::string* )NULL);
        ASSERT_EQ(a, 1);
        ASSERT_FLOAT_EQ(b, 1.1);
        ASSERT_STREQ(c.c_str(), "abc");
    }

    ::remove("abc.db");
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
