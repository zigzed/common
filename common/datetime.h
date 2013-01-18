/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef	CXX_DATETIME_H
#define CXX_DATETIME_H
#include <time.h>
#include <iosfwd>
#include "config.h"

#ifdef OS_WINDOWS
    extern "C" {
        struct timespec {
            time_t      tv_sec;		/* Seconds.  */
            long int    tv_nsec;	/* Nanoseconds.  */
        };
    }
#endif

namespace cxx {

    typedef int64_t datetime_t;
    class datetimespan;
    class datetime {
    public:
        struct calendar {
            short   year;
            short   mon;
            short   day;
            short   hour;
            short   min;
            short   sec;
            short   msec;
        };
        static const datetime now();

        datetime();
        datetime(const datetime& rhs);
        datetime(const time_t tt);
        datetime(const datetime_t& dt, bool dummy);
        datetime(short year, short mon, short day, short hour, short min, short sec, short msec);

        datetime& operator= (const datetime& rhs);
        datetime& operator= (const datetime_t& dt);
        datetime& operator= (const time_t tt);
        bool operator== (const datetime& rhs) const;
        bool operator!= (const datetime& rhs) const;
        bool operator<  (const datetime& rhs) const;
        bool operator<= (const datetime& rhs) const;
        bool operator>  (const datetime& rhs) const;
        bool operator>= (const datetime& rhs) const;

        datetime operator+ (const datetimespan& span) const;
        datetime operator- (const datetimespan& span) const;
        datetime& operator+= (const datetimespan& span);
        datetime& operator-= (const datetimespan& span);
        datetimespan operator- (const datetime& rhs) const;

        operator calendar()     const;
        operator datetime_t()   const;
        bool     valid() const;

    private:
        bool        validtime_;
        datetime_t  timestamp_;
        friend class datetimespan;
        friend std::ostream& operator<< (std::ostream& os, const datetime& dt);
    };

    class datetimespan {
    public:
        datetimespan();
        datetimespan(const datetime_t span);
        datetimespan(const datetimespan& rhs);
        datetimespan(long days, long hour, long min, long sec, long msec);

        datetimespan& operator= (const datetimespan& rhs);
        datetimespan& operator= (const datetime_t dt);
        bool operator== (const datetimespan& rhs) const;
        bool operator!= (const datetimespan& rhs) const;
        bool operator<  (const datetimespan& rhs) const;
        bool operator<= (const datetimespan& rhs) const;
        bool operator>  (const datetimespan& rhs) const;
        bool operator>= (const datetimespan& rhs) const;

        datetimespan operator-() const;
        datetimespan operator+ (const datetimespan& rhs) const;
        datetimespan operator- (const datetimespan& rhs) const;
        datetimespan& operator+= (const datetimespan& rhs);
        datetimespan& operator-= (const datetimespan& rhs);

        datetime_t getTotalDays()   const;
        datetime_t getTotalHours()  const;
        datetime_t getTotalMinutes()const;
        datetime_t getTotalSeconds()const;
        datetime_t getTotalMilliSeconds() const;

        long getDays()      const;
        long getHours()     const;
        long getMinutes()   const;
        long getSeconds()   const;
        long getMilliSeconds() const;
    private:
        datetime_t  timespan_;
        friend class datetime;
    };

}

#endif
