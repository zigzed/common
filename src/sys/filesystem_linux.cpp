/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/sys/filesystem.h"
#include "common/str/tokenizer.h"

#if defined(OS_LINUX)

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <cassert>

namespace cxx {
    namespace sys {

        namespace {
            const char* dir_sep = "/";
        }

        Path Path::curpath()
        {
            char p[PATH_MAX + 1] = {0};
            return Path(getcwd(p, PATH_MAX));
        }

        Path::Path(const char *path)
        {
            convert(path);
        }

        const char* Path::name() const
        {
            return name_.c_str();
        }

        bool Path::exist()
        {
            assert(!name_.empty());
            struct stat s;
            return stat(name_.c_str(), &s) == 0;
        }

        bool Path::isdir()
        {
            assert(!name_.empty());
            struct stat s;
            if(stat(name_.c_str(), &s) == 0) {
                return S_ISDIR(s.st_mode);
            }
            else
                return false;
        }

        bool Path::isreg()
        {
            assert(!name_.empty());
            struct stat s;
            if(stat(name_.c_str(), &s) == 0) {
                return S_ISREG(s.st_mode);
            }
            else
                return false;
        }

        bool Path::islnk()
        {
            assert(!name_.empty());
            struct stat s;
            if(stat(name_.c_str(), &s) == 0) {
                return S_ISLNK(s.st_mode);
            }
            else
                return false;
        }

        Path& Path::append(const char* path)
        {
            name_ += dir_sep;
            name_ += path;
            return *this;
        }

        void Path::convert(const std::string &name)
        {
            char abspath[PATH_MAX];
            realpath(name.c_str(), abspath);

            cxx::str::tokenizer token(abspath, dir_sep, false);

            std::vector<std::string > path;

            size_t pathsize = token.size();
            if(token[pathsize - 1].empty()) {
                pathsize--;
            }

            for(size_t i = 0; i < pathsize; ++i) {
                path.push_back(token[i]);
            }

            for(size_t i = 0; i < path.size(); ++i) {
                name_ += path[i];
                if(i != path.size() - 1) {
                    name_ += dir_sep;
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////
        DirIterator::DirIterator() : base_(Path::curpath()), pdir_(NULL), hold_(false)
        {
        }

        DirIterator::DirIterator(const char *path) : base_(path), pdir_(NULL), hold_(true)
        {
            pdir_ = opendir(path);
            next();
        }

        DirIterator::DirIterator(const Path &path) : base_(path), pdir_(NULL), hold_(true)
        {
            pdir_ = opendir(path.name());
            next();
        }

        DirIterator::DirIterator(const DirIterator &rhs) : base_(rhs.base_), hold_(true)
        {
            name_ = rhs.name_;
            pdir_ = rhs.pdir_;
            rhs.hold_ = false;
        }

        DirIterator& DirIterator::operator =(const DirIterator& rhs)
        {
            if(this == &rhs)
                return *this;

            name_ = rhs.name_;
            base_ = rhs.base_;
            pdir_ = rhs.pdir_;
            hold_ = true;
            rhs.hold_ = false;
            return *this;
        }

        DirIterator::~DirIterator()
        {
            if(pdir_) {
                closedir(pdir_);
            }
        }

        const std::string& DirIterator::name() const
        {
            return name_;
        }

        Path DirIterator::path() const
        {
            Path result(base_);
            result.append(name_.c_str());
            return result;
        }

        DirIterator& DirIterator::operator ++()
        {
            next();
            return *this;
        }

        bool DirIterator::operator ==(const DirIterator& rhs) const
        {
            return name_ == rhs.name_;
        }

        bool DirIterator::operator !=(const DirIterator& rhs) const
        {
            return name_ != rhs.name_;
        }

        const std::string &DirIterator::next()
        {
            if(pdir_ == NULL) {
                name_.clear();
                return name_;
            }
            do {
                struct dirent* entry = readdir(pdir_);
                if(entry) {
                    name_ = entry->d_name;
                }
                else
                    name_.clear();
            } while(name_ == "." || name_ == "..");

            return name_;
        }

    }
}

#endif
