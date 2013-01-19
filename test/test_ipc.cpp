/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/config.h"
#include "common/gtest/gtest.h"
#include "common/ipc/mmap.h"

TEST(FileMapping, simple)
{
    try {
        cxx::ipc::file_mapping::remove("test.map");
        cxx::ipc::file_mapping  file("test.map", cxx::ipc::memory_mappable::ReadWrite);
        file.size(8192);

        cxx::ipc::mapped_region mapping(file, cxx::ipc::mapped_region::ReadWrite);
        void* ptr = mapping.data();
        for(size_t i = 0; i < file.size() / sizeof(int); ++i) {
            *((int* )ptr + i) = i;
        }
        mapping.commit();
    }
    catch(const cxx::ipc::ipc_error& e) {
        fprintf(stderr, "ipc error: %s:%d\n", e.what(), e.code());
    }
}

TEST(FileMapping, unaligned)
{
    try {
        cxx::ipc::file_mapping::remove("test.map");
        cxx::ipc::file_mapping  file("test.map", cxx::ipc::memory_mappable::ReadWrite);
        file.size(8192);

        cxx::ipc::mapped_region mapping(file, cxx::ipc::mapped_region::ReadWrite, 1, 100, 0);
        void* ptr = mapping.data();
        ASSERT_EQ(mapping.size(), 100);
        for(size_t i = 0; i < file.size() / sizeof(int) / 2; ++i) {
            *((int* )ptr + i) = i;
        }
        mapping.commit();
    }
    catch(const cxx::ipc::ipc_error& e) {
        fprintf(stderr, "ipc error: %s:%d\n", e.what(), e.code());
    }
}

TEST(FileMapping, copy)
{
    try {
        cxx::ipc::file_mapping::remove("test.map");

        cxx::ipc::file_mapping file;
        {
            cxx::ipc::file_mapping  temp("test.map", cxx::ipc::memory_mappable::ReadWrite);
            temp.size(8192);
            file = temp;
        }

        cxx::ipc::mapped_region mapping(file, cxx::ipc::mapped_region::ReadWrite, 1, 100, 0);
        void* ptr = mapping.data();
        for(size_t i = 0; i < file.size() / sizeof(int) / 2; ++i) {
            *((int* )ptr + i) = i;
        }
        mapping.commit();
    }
    catch(const cxx::ipc::ipc_error& e) {
        fprintf(stderr, "ipc error: %s:%d\n", e.what(), e.code());
    }
}

TEST(FileMapping, performance)
{
    try {
        cxx::ipc::mapped_region::offset_t filesize = (cxx::ipc::mapped_region::offset_t)512 * 1024 * 1024;
        std::size_t mapsize = 32 * 1024;
        cxx::ipc::file_mapping::remove("test.map");
        cxx::ipc::file_mapping  file("test.map", cxx::ipc::memory_mappable::ReadWrite);
        file.size(filesize);

        for(size_t i = 0; i < filesize / mapsize; ++i) {
            cxx::ipc::mapped_region mapping(file, cxx::ipc::mapped_region::ReadWrite, i * mapsize, mapsize);
            mapping.advise(cxx::ipc::mapped_region::Sequential);
            void* ptr = mapping.data();
            for(size_t i = 0; i < mapsize / sizeof(int); ++i) {
                *((int* )ptr + i) = i;
            }
            mapping.commit();
        }
    }
    catch(const cxx::ipc::ipc_error& e) {
        fprintf(stderr, "ipc error: %s:%d\n", e.what(), e.code());
    }
}

TEST(FileMapping, move)
{
    try {
        cxx::ipc::mapped_region::offset_t filesize = (cxx::ipc::mapped_region::offset_t)512 * 1024 * 1024;
        cxx::ipc::file_mapping::remove("test.map");
        cxx::ipc::file_mapping  file("test.map", cxx::ipc::memory_mappable::ReadWrite);
        file.size(filesize);

        std::size_t mapsize = 32 * 1024;
        cxx::ipc::mapped_region mapping;
        mapping.attach(file, cxx::ipc::mapped_region::ReadWrite);

        for(size_t i = 0; i < filesize / mapsize; ++i) {
            mapping.move(i * mapsize, mapsize);
            mapping.advise(cxx::ipc::mapped_region::Sequential);
            void* ptr = mapping.data();
            for(size_t i = 0; i < mapsize / sizeof(int); ++i) {
                *((int* )ptr + i) = i;
            }
            mapping.commit();
        }
    }
    catch(const cxx::ipc::ipc_error& e) {
        fprintf(stderr, "ipc error: %s:%d\n", e.what(), e.code());
    }
}


TEST(SharedMemory, simple)
{
    try {
        cxx::ipc::shared_memory::remove("test.map");
        cxx::ipc::shared_memory  file("test.map", cxx::ipc::memory_mappable::ReadWrite);
        file.size(8192);

        cxx::ipc::mapped_region mapping(file, cxx::ipc::mapped_region::ReadWrite);
        void* ptr = mapping.data();
        for(size_t i = 0; i < file.size() / sizeof(int); ++i) {
            *((int* )ptr + i) = i;
        }
        mapping.commit();
    }
    catch(const cxx::ipc::ipc_error& e) {
        fprintf(stderr, "ipc error: %s:%d\n", e.what(), e.code());
    }
}

TEST(SharedMemory, unaligned)
{
    try {
        cxx::ipc::shared_memory::remove("test.map");
        cxx::ipc::shared_memory  file("test.map", cxx::ipc::memory_mappable::ReadWrite);
        file.size(8192);

        cxx::ipc::mapped_region mapping(file, cxx::ipc::mapped_region::ReadWrite, 1, 100, 0);
        void* ptr = mapping.data();
        ASSERT_EQ(mapping.size(), 100);
        for(size_t i = 0; i < file.size() / sizeof(int) / 2; ++i) {
            *((int* )ptr + i) = i;
        }
        mapping.commit();
    }
    catch(const cxx::ipc::ipc_error& e) {
        fprintf(stderr, "ipc error: %s:%d\n", e.what(), e.code());
    }
}

TEST(SharedMemory, copy)
{
    try {
        cxx::ipc::shared_memory::remove("test.map");

        cxx::ipc::shared_memory file;
        {
            cxx::ipc::shared_memory  temp("test.map", cxx::ipc::memory_mappable::ReadWrite);
            temp.size(8192);
            file = temp;
        }

        cxx::ipc::mapped_region mapping(file, cxx::ipc::mapped_region::ReadWrite, 1, 100, 0);
        void* ptr = mapping.data();
        for(size_t i = 0; i < file.size() / sizeof(int) / 2; ++i) {
            *((int* )ptr + i) = i;
        }
        mapping.commit();
    }
    catch(const cxx::ipc::ipc_error& e) {
        fprintf(stderr, "ipc error: %s:%d\n", e.what(), e.code());
    }
}

TEST(SharedMemory, performance)
{
    try {
        cxx::ipc::mapped_region::offset_t filesize = (cxx::ipc::mapped_region::offset_t)512 * 1024 * 1024;
        std::size_t mapsize = 32 * 1024;
        cxx::ipc::shared_memory::remove("test.map");
        cxx::ipc::shared_memory  file("test.map", cxx::ipc::memory_mappable::ReadWrite);
        file.size(filesize);

        for(size_t i = 0; i < filesize / mapsize; ++i) {
            cxx::ipc::mapped_region mapping(file, cxx::ipc::mapped_region::ReadWrite, i * mapsize, mapsize);
            mapping.advise(cxx::ipc::mapped_region::Sequential);
            void* ptr = mapping.data();
            for(size_t i = 0; i < mapsize / sizeof(int); ++i) {
                *((int* )ptr + i) = i;
            }
            mapping.commit();
        }
    }
    catch(const cxx::ipc::ipc_error& e) {
        fprintf(stderr, "ipc error: %s:%d\n", e.what(), e.code());
    }
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
