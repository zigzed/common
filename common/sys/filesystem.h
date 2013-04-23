/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_SYS_FILESYSTEM_H
#define CXX_SYS_FILESYSTEM_H
#include <string>
#include "common/config.h"
#if     defined(OS_LINUX)
    #include <dirent.h>
#elif   defined(OS_WINDOWS)
	#include <Windows.h>
#endif

namespace cxx {
    namespace sys {

        class Path {
        public:
            static Path curpath();

            Path(const char* path);
            /// normalized path name
            const char* name() const;
            /// is it exist?
            bool        exist();
            /// is it a regular file?
            bool        isreg();
            /// is it a directory?
            bool        isdir();
            /// is it a symbolic link?
            bool        islnk();

            Path& append(const char* rhs);

            bool operator< (const Path& rhs) const;
            bool operator==(const Path& rhs) const;
        private:
            void convert(const std::string& name);
            std::string name_;
        };

        class DirIterator {
        public:
            DirIterator();
            DirIterator(const char* path);
            DirIterator(const Path& path);
            DirIterator(const DirIterator& rhs);
            DirIterator& operator= (const DirIterator& rhs);
            ~DirIterator();

            const std::string&  name() const;
            Path                path() const;

            DirIterator& operator++();

            bool operator==(const DirIterator& rhs) const;
            bool operator!=(const DirIterator& rhs) const;
        private:
            const std::string& next();
            std::string name_;
            Path        base_;
#if     defined(OS_LINUX)
            DIR*        pdir_;
#elif   defined(OS_WINDOWS)
			HANDLE hFile_;
			WIN32_FIND_DATA FindFileData_;
#endif
            mutable bool hold_;
        };

    }
}

#endif
