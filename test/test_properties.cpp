/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/cfg/properties.h"
#include "common/sys/filesystem.h"

void setup_cfg1(const char* file)
{
    std::ofstream ofs(file, std::ios_base::out | std::ios_base::trunc | std::ios_base::ate);
    ofs << "t1${TEST_ENV}=1\n"
        << "t2.${TEST_ENV}.y=2\n"
        << "${TEST_ENV} =3\n"
        << "t3=4${TEST_ENV}\n"
        << "t4${TEST}=5\n"
        << "t5${TEST_ENV}m=p${TEST_ENV}p\n"
        << "t6${TEST}y=6\n";
    ofs.close();
}

void setup_cfg2(const char* file)
{
    std::ofstream ofs(file, std::ios_base::out | std::ios_base::trunc | std::ios_base::ate);
    ofs << "dsa.${INSTANCE}.thread = 4\n"
        << "dsa.${INSTANCE}.port = 8000 + ${INSTANCE}\n"
        << "dsa.1.overlap = 1\n"
        << "dsa.${INSTANCE}.overlap = 0\n"
        << "dsa.9.overlap = 9\n"
        << "include = ${INSTANCE}.conf\n"
           ;
    ofs.close();
}

void setup_env(const char* key, const char* val)
{
    if(getenv(key) != NULL) {
        printf("TEST_ENV is defined, use another environment variable name\n");
    }

#ifdef OS_WINDOWS
		char buf[1024] = {0};
        sprintf(buf,"%s=%s",key,val);
		putenv(buf);
#else
        setenv(key, val, 1);
#endif
}

void erase_cfg(const char* file)
{
    unlink(file);
}

TEST(properties, simple)
{
    setup_env("TEST_ENV", "x");
    setup_cfg1("test.cfg");

    cxx::cfg::properties cfg;
    cfg.load("test.cfg");

    ASSERT_EQ("1", cfg.getProperty("t1x"));
    ASSERT_EQ("2", cfg.getProperty("t2.x.y"));
    ASSERT_EQ("3", cfg.getProperty("x"));
    ASSERT_EQ("4x", cfg.getProperty("t3"));
    ASSERT_EQ("5", cfg.getProperty("t4${TEST}"));
    ASSERT_EQ("pxp", cfg.getProperty("t5xm"));
    ASSERT_EQ("6", cfg.getProperty("t6${TEST}y"));

    erase_cfg("test.cfg");
}

TEST(properties, calc)
{
    {
        setup_env("INSTANCE", "1");
        setup_cfg2("test.cfg");

        cxx::cfg::properties cfg;
        cfg.load("test.cfg");

        ASSERT_EQ("4", cfg.getProperty("dsa.1.thread"));
        ASSERT_EQ("8001", cfg.getProperty("dsa.1.port"));
        ASSERT_EQ("1", cfg.getProperty("dsa.1.overlap"));

        erase_cfg("test.cfg");
    }

    {
        setup_env("INSTANCE", "2");
        setup_cfg2("test.cfg");

        cxx::cfg::properties cfg;
        cfg.load("test.cfg");

        ASSERT_EQ("4", cfg.getProperty("dsa.2.thread"));
        ASSERT_EQ("8002", cfg.getProperty("dsa.2.port"));
        ASSERT_EQ("0", cfg.getProperty("dsa.2.overlap"));

        erase_cfg("test.cfg");
    }

}

TEST(properties, bug1)
{
    cxx::cfg::properties cfg;
    cfg.load("../../test/test_properties.conf");

    ASSERT_EQ(cfg.getProperty("dsc.conf.basedir"), "E:\\GanSuMaster\\dsc_abis\\conf\\");
    ASSERT_EQ(cfg.getProperty("output.file"), "output_ydxj.json");
    ASSERT_EQ(cfg.getProperty("log.dsc.file.name"), "dsc-summary.log");
    ASSERT_EQ(cfg.getProperty("bus.nameserver"), "tcp://192.168.13.112:4455");
    ASSERT_EQ(cfg.getProperty("bus.sub.dec.host"), "tcp://:4456");
}

TEST(properties, bug2)
{
    cxx::sys::Path base("../../test/etc");
    cxx::sys::DirIterator dir(base);
    while(dir != cxx::sys::DirIterator()) {
        cxx::sys::Path temp(base);
        temp.append(dir.name().c_str());
        printf("loading %s\n", temp.name());
        cxx::cfg::properties cfg;
        cfg.load(temp.name());
        cfg.getProperty("abc");
        ++dir;
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
