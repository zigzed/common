/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "filelog.h"
#include <stdio.h>
#include <string.h>

#if defined(OS_WINDOWS)
	#define	snprintf	_snprintf
#endif

namespace cxx {
    namespace log {

        static const char* level(LogLevel level)
        {
            static const char* message[] = {
                "emerg", "alert", "crit", "error",
                "warn", "notice", "info", "debug"
            };
            if(level >= 0 && level < sizeof(message)/sizeof(message[0])) {
                return message[level];
            }
            return "unknown";
        }

        FileChannel::FileChannel(LogLevel level, const char *filename, size_t size)
            : faci_(level), file_(NULL), size_(0), vols_(size), name_(filename)
        {
            file_ = ::fopen(name_.c_str(), "a+");
            ::fseek(file_, 0, SEEK_END);
            curr_= ::ftell(file_);
            if(curr_ == -1) {
                curr_ = 0;
            }
        }

        void FileChannel::on_message(const LogInfo &info)
        {
            if(info.faci > faci_) {
                return;
            }
            char			buff[2048];
            cxx::datetime::calendar cur(info.time);
            char*           p = buff;
            int             size = sizeof(buff);
            while(true) {
                int count = snprintf(buff, size, "%04d%02d%02d %02d%02d%02d.%03d <%s> [%s:%d:%s] %s\n",
                                     cur.year, cur.mon, cur.day, cur.hour, cur.min, cur.sec, cur.msec,
                                     level(info.faci), info.file, info.line, info.func, info.buff);
                if(count > -1 && count < size) {
                    cxx::sys::plainmutex::scopelock mutex(lock_);
                    size_t len = fwrite(p, count, 1, file_);
                    if(len != 1) {
                        this->dclose();
                        file_ = fopen(name_.c_str(), "a+");
                        fseek(file_, 0, SEEK_END);
                        curr_ = ftell(file_);
                        if(curr_ == -1) {
                            curr_ = 0;
                        }
                    }
                    else {
                        curr_ += count;
                        if(curr_ > vols_) {
                            rotate();
                        }
                    }
                    break;
                }
                else {
                    if(count > 0) {
                        size = count + 1;
                    }
                    else {
                        size *= 2;
                    }
                    if(p != buff) {
                        delete[] p;
                    }
                    p = new char[size];
                }
            }
            if(info.faci <= cxx::log::error) {
                fflush(file_);
            }

            if(p != buff) {
                delete[] p;
            }
        }

        FileChannel::~FileChannel()
        {
            this->dclose();
        }

        void FileChannel::dclose()
        {
            if(file_) {
                fclose(file_);
                file_ = NULL;
            }
        }

        void FileChannel::rotate()
        {
            static const size_t	MaxBackupIndex	= 100;
            this->dclose();
            char	oldname[256]	= {0};
            char	newname[256]	= {0};

            sprintf(oldname, "%s.%03ld", name_.c_str(), MaxBackupIndex);
            ::remove(oldname);
            for(size_t i = MaxBackupIndex; i > 1; i--) {
                strcpy(newname, oldname);
                sprintf(oldname, "%s.%03ld", name_.c_str(), i - 1);
                ::rename(oldname, newname);
            }
            ::rename(name_.c_str(), oldname);

            file_ = ::fopen(name_.c_str(), "a+");
            curr_= 0;
        }


    }
}
