/** Copyright (C) 2013 wilburlang@gmail.com
 */
#include <string.h>
#include <ostream>
#include <iomanip>
#include <cassert>
#include "common/config.h"
#include "common/datetime.h"
#if defined(OS_LINUX)
    #include <sys/time.h>
#endif

namespace cxx {

#if defined(_MSC_VER) || defined(__BORLANDC__)
    #define LLCONST(a)  (a##i64)
#else
    #define LLCONST(a)  (a##ll)
#endif

    class TimeZone {
    public:
        TimeZone() : offset_(0) {}
        ~TimeZone() { delete offset_; }
        long offset();
    private:
        long* offset_;
    };

    long TimeZone::offset()
    {
        if(offset_ == 0) {
            offset_ = new long;
            *offset_= 0;
#ifdef  __USE_BSD
            time_t  t0;
            tm      t1;
            time(&t0);
            localtime_r(&t0, &t1);
            *offset_ = t1.tm_gmtoff / 60;
#endif
        }
        return *offset_;
    }

    // const array with months number of days of year
    static const short MonthDayInYear[] = {0, 31, 59, 90, 120, 151, 181, 212,
                                            243, 273, 304, 334, 365 };

    static datetime makedate(short year, short mon, short day, short hour, short min, short sec, short msec)
    {
        assert(year <= 29000);
        assert(year >= -29000);
        assert(mon >= 1 && mon <= 12);
        assert(day >= 1 && day <= 31);

        bool isLeapYear = ((year & 3) == 0) && ((year % 100) != 0 || (year % 400) == 0);

        int imsec   = msec;
        int isec    = sec;
        int imin    = min;
        int ihour   = hour;
        int iday    = day;

        isec    += imsec / 1000;
        imsec   %= 1000;
        imin    += isec / 60;
        isec    %= 60;
        ihour   += imin / 60;
        imin    %= 60;
        iday    += ihour / 24;
        ihour   %= 24;

        // it's a valid date; make Jan 1, 1AD be 1
        datetime_t temp = year * 365L + year / 4 - year / 100 + year / 400 + MonthDayInYear[mon - 1] + iday;
        // if leap year and it's before March, substract 1
        if(mon <= 2 && isLeapYear)
            --temp;

        // offset so that 01/01/1970 is 0
        temp -= 719528L;
        // change date to seconds
        temp *= 86400L;
        temp += (ihour * 3600L) + (imin * 60L) + isec;
        // change date to milliseconds
        temp *= 1000L;
        temp += imsec;

        return datetime(datetime_t(temp), false);
    }

    static datetime makedate(long days, long msecs)
    {
        datetime_t temp = days;
        // change date to seconds
        temp *= LLCONST(86400);
        temp *= LLCONST(1000);
        temp += msecs;

        return datetime(datetime_t(temp), false);
    }

    static datetime makedate(const timeval& tv)
    {
        datetime_t temp = tv.tv_sec;
        temp *= 1000;
        temp += tv.tv_usec / 1000;
        return datetime(datetime_t(temp), false);
    }

    static datetime::calendar makecalendar(datetime_t src)
    {
        datetime_t temp = src;
        long daysAbsolute;  // number of days since 1/1/0
        long secInDay;      // time in seconds since midnight
        long minInDay;      // minutes in day
        long n400Years;     // number of 400 year increments since 1/1/0
        long n400Century;   // century within 400 year block
        long n4Years;       // number of 4 year increments since 1/1/0
        long n4Day;         // day within 4 year
        long n4Yr;          // Year within 4 year
        bool leap = true;   // 4 year block includes leap year
        long milliThisDay;

        milliThisDay= (long)(src % 1000L);
        temp       /= 1000L;
        secInDay    = (long)(temp % 86400L);
        temp       /= 86400L;
        daysAbsolute= (long)(temp);
        daysAbsolute+= 719528L; // add days from 1/1/0 to 1/1/1970

        // leap years every 4 yrs except centruies not multiples of 400
        n400Years = (long)(daysAbsolute / 146097L);
        // set daysAbsolute to day within 400-year block
        daysAbsolute %= 146097L;
        // -1 because first century has extra day
        n400Century = (long)((daysAbsolute - 1) / 36524L);

        // non-leap century
        if(n400Century != 0) {
            daysAbsolute= (daysAbsolute - 1) % 36524L;
            n4Years     = (long)((daysAbsolute + 1) / 1461L);
            if(n4Years != 0)
                n4Day   = (long)((daysAbsolute + 1) % 1461L);
            else {
                leap    = false;
                n4Day   = (long)daysAbsolute;
            }
        }
        else {
            n4Years = (long)(daysAbsolute / 1461L);
            n4Day   = (long)(daysAbsolute % 1461L);
        }

        if(leap) {
            n4Yr    = (n4Day - 1) / 365;
            if(n4Yr != 0)
                n4Day= (n4Day - 1) % 365;
        }
        else {
            n4Yr    = n4Day / 365;
            n4Day   %= 365;
        }

        datetime::calendar    cad;
        cad.year    = n400Years * 400 + n400Century * 100 + n4Years * 4 + n4Yr;
        if(n4Yr == 0 && leap) {
            if(n4Day == 59) {
                cad.mon = 2;
                cad.day = 29;
                goto DoTime;
            }

            if(n4Day >= 60)
                --n4Day;
        }

        ++n4Day;
        for(cad.mon = (n4Day >> 5) + 1; n4Day > MonthDayInYear[cad.mon]; cad.mon++);
        cad.day = (short)(n4Day - MonthDayInYear[cad.mon - 1]);

DoTime:
        if(secInDay == 0)
            cad.hour    = cad.min = cad.sec = 0;
        else {
            cad.sec     = (short)(secInDay % 60L);
            minInDay    = secInDay / 60L;
            cad.min     = (short)(minInDay % 60);
            cad.hour    = (short)(minInDay / 60);
        }

        cad.msec        = milliThisDay;

        return cad;
    }
    ///////////////////////////////////////////////////////////////////////////
    const datetime datetime::now()
    {
#if defined(OS_WINDOWS)
        SYSTEMTIME  t;
        ::GetLocalTime(&t);
        return makedate(t.wYear, t.wMonth, t.wDay,
                        t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
#else
        timeval tv;
        ::gettimeofday(&tv, 0);
        static TimeZone	timeZone;
        /// days since unix "epoch" 1970-01-01 plus days between 1601-01-01 to unix "epoch"
        tv.tv_sec += timeZone.offset() * 60;
        return makedate(tv);
#endif
    }

    datetime::datetime(short year, short mon, short day, short hour, short min, short sec, short msec)
    {
        *this = makedate(year, mon, day, hour, min, sec, msec);
    }

    datetime::datetime(const time_t tt)
    {
        tm  lt;
#ifdef  OS_WINDOWS
        ::localtime_s(&lt, &tt);
#else
        ::localtime_r(&tt, &lt);
#endif
        *this = makedate(lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday, lt.tm_hour, lt.tm_min, lt.tm_sec, 0);
    }

    datetime datetime::operator + (const datetimespan& span) const
    {
        datetime temp = *this;
        temp.timestamp_ += span.timespan_;
        return temp;
    }

    datetime datetime::operator - (const datetimespan& span) const
    {
        datetime temp = *this;
        temp.timestamp_ -= span.timespan_;
        return temp;
    }

    datetime& datetime::operator += (const datetimespan& span)
    {
        timestamp_ += span.timespan_;
        return *this;
    }

    datetime& datetime::operator -= (const datetimespan& span)
    {
        timestamp_ -= span.timespan_;
        return *this;
    }

    datetimespan datetime::operator - (const datetime& rhs) const
    {
        datetime_t temp = timestamp_ - rhs.timestamp_;
        return temp;
    }

    datetime::operator datetime::calendar() const
    {
        return makecalendar(timestamp_);
    }

    datetime::operator datetime_t() const
    {
        return timestamp_;
    }

    bool datetime::valid() const
    {
        return validtime_;
    }

    ///////////////////////////////////////////////////////////////////////////
    datetime_t datetimespan::getTotalDays() const
    {
        datetime_t temp;
        temp  = timespan_ / 1000L;
        temp /= 86400L;

        return temp;
    }

    datetime_t datetimespan::getTotalHours() const
    {
        datetime_t temp;
        temp  = timespan_ / 1000L;
        temp /= 3600L;
        return temp;
    }

    datetime_t datetimespan::getTotalMinutes() const
    {
        datetime_t temp;
        temp  = timespan_ / 1000L;
        temp /= 60L;
        return temp;
    }

    datetime_t datetimespan::getTotalSeconds() const
    {
        datetime_t temp;
        temp  = timespan_ / 1000L;
        return temp;
    }

    datetime_t datetimespan::getTotalMilliSeconds() const
    {
        return timespan_;
    }

    long datetimespan::getDays() const
    {
        datetime_t temp;
        temp  = timespan_ / 1000L;
        temp /= 86400L;
        return (long)temp;
    }

    long datetimespan::getHours() const
    {
        datetime_t temp;
        temp  = timespan_ / 1000L;
        temp  = (temp % 86400L) / 3600L;
        return (long)temp;
    }

    long datetimespan::getMinutes() const
    {
        datetime_t temp;
        temp = timespan_ / 1000L;
        temp = (temp % 3600L) / 60L;
        return (long)temp;
    }

    long datetimespan::getSeconds() const
    {
        datetime_t temp;
        temp = timespan_ / 1000L;
        temp = (temp % 60L);
        return (long)temp;
    }

    long datetimespan::getMilliSeconds() const
    {
        datetime_t temp;
        temp = timespan_ % 1000L;
        return (long)temp;
    }

    ///////////////////////////////////////////////////////////////////////////
    datetime::datetime() : validtime_(false), timestamp_(0)
    {
    }

    datetime::datetime(const datetime& rhs) : validtime_(rhs.validtime_), timestamp_(rhs.timestamp_)
    {
    }

    datetime::datetime(const datetime_t& dt, bool /*dummy*/) : validtime_(true), timestamp_(dt)
    {
    }

    datetime& datetime::operator= (const datetime& rhs)
    {
        if(this == &rhs)
            return *this;
        validtime_ = rhs.validtime_;
        timestamp_ = rhs.timestamp_;
        return *this;
    }

    datetime& datetime::operator= (const datetime_t& dt)
    {
        validtime_ = true;
        timestamp_ = dt;
        return *this;
    }

    bool datetime::operator==(const datetime& rhs) const
    {
        return ((validtime_ == rhs.validtime_) && (timestamp_ == rhs.timestamp_));
    }

    bool datetime::operator!= (const datetime& rhs) const
    {
        return ((validtime_ != rhs.validtime_) || (timestamp_ != rhs.timestamp_));
    }

    bool datetime::operator < (const datetime& rhs) const
    {
        assert(validtime_ && rhs.validtime_);
        return timestamp_ < rhs.timestamp_;
    }

    bool datetime::operator<= (const datetime& rhs) const
    {
        assert(validtime_ && rhs.validtime_);
        return timestamp_ <= rhs.timestamp_;
    }

    bool datetime::operator>  (const datetime& rhs) const
    {
        assert(validtime_ && rhs.validtime_);
        return timestamp_ > rhs.timestamp_;
    }

    bool datetime::operator>= (const datetime& rhs) const
    {
        assert(validtime_ && rhs.validtime_);
        return timestamp_ >= rhs.timestamp_;
    }

    ///////////////////////////////////////////////////////////////////////////
    datetimespan::datetimespan() : timespan_(0)
    {
    }

    datetimespan::datetimespan(long ms) : timespan_(ms)
    {
    }

    datetimespan::datetimespan(long days, long hour, long min, long sec, long msec)
    {
        sec     += msec / 1000;
        msec    %= 1000;
        min     += sec / 60;
        sec     %= 60;
        hour    += min / 60;
        min     %= 60;
        days    += hour / 24;
        hour    %= 24;

        timespan_  = days;
        timespan_ *= 86400L;
        timespan_ += (hour * 3600L) + (min * 60L) + sec;
        timespan_ *= 1000L;
        timespan_ += msec;
    }

    datetimespan& datetimespan::operator = (const datetimespan& rhs)
    {
        if(this == &rhs)
            return *this;
        timespan_ = rhs.timespan_;
        return *this;
    }

    datetimespan& datetimespan::operator = (const datetime_t dt)
    {
        timespan_ = dt;
        return *this;
    }

    bool datetimespan::operator == (const datetimespan& rhs) const
    {
        return timespan_ == rhs.timespan_;
    }

    bool datetimespan::operator != (const datetimespan& rhs) const
    {
        return timespan_ != rhs.timespan_;
    }

    bool datetimespan::operator <  (const datetimespan& rhs) const
    {
        return timespan_ < rhs.timespan_;
    }

    bool datetimespan::operator <= (const datetimespan& rhs) const
    {
        return timespan_ <= rhs.timespan_;
    }

    bool datetimespan::operator >  (const datetimespan& rhs) const
    {
        return timespan_ > rhs.timespan_;
    }

    bool datetimespan::operator >= (const datetimespan& rhs) const
    {
        return timespan_ >= rhs.timespan_;
    }

    datetimespan& datetimespan::operator +=(const datetimespan& span)
    {
        timespan_ += span.timespan_;
        return *this;
    }

    datetimespan& datetimespan::operator -=(const datetimespan& span)
    {
        timespan_ -= span.timespan_;
        return *this;
    }

    datetimespan datetimespan::operator -() const
    {
        return -timespan_;
    }

    std::ostream& operator<< (std::ostream& os, const datetime& dt)
    {
        cxx::datetime::calendar cad = (cxx::datetime::calendar)dt;
        os << std::setw(4) << std::setfill('0') << cad.year << "-"
            << std::setw(2) << std::setfill('0') << cad.mon << "-"
            << std::setw(2) << std::setfill('0') << cad.day << " "
            << std::setw(2) << std::setfill('0') << cad.hour << ":"
            << std::setw(2) << std::setfill('0') << cad.min << ":"
            << std::setw(2) << std::setfill('0') << cad.sec << "."
            << std::setw(3) << std::setfill('0') << cad.msec;

        return os;
    }

}


