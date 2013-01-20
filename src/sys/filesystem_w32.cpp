/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "common/sys/filesystem.h"
#include "common/str/tokenizer.h"
#include <cassert>

#if defined(OS_WINDOWS)
#include <Windows.h>

namespace cxx {
    namespace sys {

        namespace {
            const char* dir_sep = "\\";
        }

        Path Path::curpath()
        {
            char p[MAX_PATH + 1] = {0};
            GetCurrentDirectory(MAX_PATH, p);
            return Path(p);
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
            DWORD attr = GetFileAttributes(name_.c_str());
            if(attr == 0xFFFFFFFF) {
                switch(GetLastError()) {
                case ERROR_FILE_NOT_FOUND:
                case ERROR_PATH_NOT_FOUND:
                case ERROR_NOT_READY:
                case ERROR_INVALID_DRIVE:
                    return false;
                default:
                    return false;
                }
            }
            return true;
        }

        bool Path::isdir()
        {
            assert(!name_.empty());
            DWORD attr = GetFileAttributes(name_.c_str());
            if(attr == 0xFFFFFFFF) {
                return false;
            }
            return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
        }

        bool Path::isreg()
        {

			return false;
        }

        bool Path::islnk()
        {
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
            char abspath[MAX_PATH];
            unsigned int len = GetFullPathName(name.c_str(), MAX_PATH, abspath, NULL);

            cxx::StringTokenizer token(std::string(abspath, len), dir_sep, false);

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
		DirIterator::DirIterator() : hFile_(NULL), hold_(false)
		{
		}

		DirIterator::DirIterator(const char *path) : hFile_(NULL), hold_(true)
		{
			char buffFn[1024] = "";
			sprintf(buffFn, "%s\\*.*", path );

    		hFile_ = FindFirstFile( buffFn , &FindFileData_);

			next();
		}

		DirIterator::DirIterator(const Path &path) : hFile_(NULL), hold_(true)
		{
			char buffFn[1024] = "";
			sprintf(buffFn, "%s\\*.*", path.name() );

			hFile_ = FindFirstFile( buffFn , &FindFileData_);
			next();
		}

    DirIterator::DirIterator(const DirIterator &rhs) : hold_(true)
    {
        name_ = rhs.name_;
        hFile_ = rhs.hFile_;
        rhs.hold_ = false;
    }

    DirIterator& DirIterator::operator =(const DirIterator& rhs)
    {
        if(this == &rhs)
            return *this;

        name_ = rhs.name_;
        hFile_ = rhs.hFile_;
        hold_ = true;
        rhs.hold_ = false;
        return *this;
    }

		DirIterator::~DirIterator()
		{
			if(hFile_) {
				FindClose(hFile_);
			}
		}

		const std::string& DirIterator::name() const
		{
			return name_;
		}

		Path DirIterator::path() const
		{
			return Path(name_.c_str());
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
			static bool fflag = true;
			if(hFile_ == NULL || !fflag) {
				name_.clear();
				fflag = true;
				return name_;
			}
			do {
				name_.clear();
				if(fflag == true) {
					name_ = FindFileData_.cFileName;
				}
				else
					name_.clear();
				fflag = FindNextFile(hFile_, &FindFileData_);
				
			} while(name_ == "." || name_ == "..");
			return name_;
		}

    }
}

#endif
