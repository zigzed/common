/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include "logger.h"
#include <string.h>
#include <stdexcept>
#include <iostream>
#include <vector>
#include "coutlog.h"
#include "filelog.h"
#include "sloglog.h"
#include "common/cfg/properties.h"
#include "common/str/tokenizer.h"

namespace cxx {
    namespace log {

        LogManager* LogManager::get()
        {
            static LogManager* instance = NULL;
            if(instance == NULL) {
                instance = new LogManager();
            }
            return instance;
        }

        void LogManager::attach(const char *module, LogChannel *channel)
        {
            cxx::sys::plainmutex::scopelock lock(locker_);

            // TODO: 前缀匹配算法需要调整
            ChannelInfo::iterator it = device_.begin();
            for(; it != device_.end(); ++it) {
            	#ifdef OS_WINDOWS
                size_t len = min(strlen(module), it->first.size());
                #else
                size_t len = std::min(strlen(module), it->first.size());
                #endif
                if(strncmp(module, it->first.c_str(), len) == 0 && channel) {
                    it->second.insert(channel);
                }
            }

            ChannelInfo::iterator ft = device_.find(module);
            if(ft != device_.end()) {
                if(channel) {
                    ft->second.insert(channel);
                }
            }
            else {
                ChannelList list;
                if(channel) {
                    list.insert(channel);
                }
                device_[module] = list;
            }
        }

        void LogManager::detach(const char *module, LogChannel *channel)
        {
            cxx::sys::plainmutex::scopelock lock(locker_);

            ChannelInfo::iterator it = device_.begin();
            while(it != device_.end()) {
                ChannelList& list = it->second;
                list.erase(channel);
                if(list.empty()) {
                    device_.erase(it++);
                }
                else {
                    ++it;
                }
            }
        }

        void LogManager::logger(const char* module, const LogInfo &info)
        {
            ChannelList devs;
            {
                cxx::sys::plainmutex::scopelock lock(locker_);
                {
                    // 如果主题没有登记过，那么自动登记
                    ChannelInfo::iterator it = device_.find(module);
                    if(it == device_.end()) {
                        attach(module, NULL);
                    }
                }

                ChannelInfo::iterator it = device_.find(module);
                if(it != device_.end()) {
                    devs = it->second;
                }
            }

            for(ChannelList::iterator it = devs.begin(); it != devs.end(); ++it) {
                LogChannel* dev = *it;
                if(dev) {
                    dev->on_message(info);
                }
            }
        }

        static std::vector<std::string > get_config(const std::string& line)
        {
            std::vector<std::string >   result;
            cxx::str::tokenizer token(line, " :,");
            for(size_t i = 0; i < token.size(); ++i) {
                result.push_back(token[i]);
            }
            return result;
        }

        static bool has_config(const std::vector<std::string >& cfgs, const std::string& cfg)
        {
            for(size_t i = 0; i < cfgs.size(); ++i) {
                if(cfg == cfgs[i]) {
                    return true;
                }
            }
            return false;
        }

        static Facility get_facility(const std::string& faci)
        {
            if(faci == "kern")
                return kern;
            else if(faci == "user")
                return user;
            else if(faci == "mail")
                return mail;
            else if(faci == "daemon")
                return daemon;
            else if(faci == "auth")
                return auth;
            else if(faci == "syslog")
                return syslog;
            else if(faci == "lpr")
                return lpr;
            else if(faci == "news")
                return news;
            else if(faci == "uucp")
                return uucp;
            else if(faci == "cron")
                return cron;
            else if(faci == "authpriv")
                return authpriv;
            else if(faci == "ftp")
                return ftp;
            else if(faci == "local0")
                return local0;
            else if(faci == "local1")
                return local1;
            else if(faci == "local2")
                return local2;
            else if(faci == "local3")
                return local3;
            else if(faci == "local4")
                return local4;
            else if(faci == "local5")
                return local5;
            else if(faci == "local6")
                return local6;
            else if(faci == "local7")
                return local7;
            else
                return local0;
        }

        void LogInit(const char* file, const char* module)
        {
            try {
                cxx::cfg::properties config;
                config.load(file);

                std::string prefix = "log.";
                prefix += module;

                std::string output = config.getProperty(prefix + ".output");
                std::vector<std::string > outputs = get_config(output);
                if(has_config(outputs, "file")) {
                    std::string level = config.getProperty(prefix + ".file.level");
                    std::vector<std::string > levels = get_config(level);
                    LogLevel logLevel = error;
                    if(has_config(levels, "debug")) {
                        logLevel = debug;
                    }
                    else if(has_config(levels, "info")) {
                        logLevel = info;
                    }
                    else if(has_config(levels, "notice")) {
                        logLevel = notice;
                    }
                    else if(has_config(levels, "warning")) {
                        logLevel = warning;
                    }
                    else {
                        logLevel = error;
                    }
                    std::string logfilename = config.getProperty(prefix + ".file.name");
                    int         logfilesize = config.getPropertyAsIntWithDefault(prefix + "file.size", 100 * 1024 * 1024);
                    FileChannel* filechannel= new FileChannel(logLevel, logfilename.c_str(), logfilesize);
                    LogManager* mgr = LogManager::get();
                    mgr->attach(module, filechannel);
                }
                if(has_config(outputs, "cout")) {
                    std::string level = config.getProperty(prefix + ".cout.level");
                    std::vector<std::string > levels = get_config(level);
                    LogLevel logLevel = error;
                    if(has_config(levels, "debug")) {
                        logLevel = debug;
                    }
                    else if(has_config(levels, "info")) {
                        logLevel = info;
                    }
                    else if(has_config(levels, "notice")) {
                        logLevel = notice;
                    }
                    else if(has_config(levels, "warning")) {
                        logLevel = warning;
                    }
                    else {
                        logLevel = error;
                    }
                    static CoutChannel* coutchannel = NULL;
                    if(coutchannel == NULL) {
                        coutchannel= new CoutChannel(logLevel);
                    }
                    LogManager* mgr = LogManager::get();
                    mgr->attach(module, coutchannel);
                }
                if(has_config(outputs, "slog")) {
                    std::string level = config.getProperty(prefix + ".slog.level");
                    std::vector<std::string > levels = get_config(level);
                    LogLevel logLevel = error;
                    if(has_config(levels, "debug")) {
                        logLevel = debug;
                    }
                    else if(has_config(levels, "info")) {
                        logLevel = info;
                    }
                    else if(has_config(levels, "notice")) {
                        logLevel = notice;
                    }
                    else if(has_config(levels, "warning")) {
                        logLevel = warning;
                    }
                    else {
                        logLevel = error;
                    }
                    std::string loghostname = config.getProperty(prefix + ".slog.host");
                    int         loghostsize = config.getPropertyAsIntWithDefault(prefix + "slog.port", 514);
                    std::string logfacility = config.getPropertyWithDefault(prefix + ".slog.facility", "local0");
                    Facility faci = get_facility(logfacility);
                    SlogChannel* slogchannel= new SlogChannel(logLevel, module, faci, loghostname.c_str(), loghostsize);
                    LogManager* mgr = LogManager::get();
                    mgr->attach(module, slogchannel);
                }
            }
            catch(const std::runtime_error& e) {
                std::cerr << "LogInit(" << file << "," << module << ") failed: file not exist\n";
            }
        }

    }
}
