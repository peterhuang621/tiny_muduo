#pragma once

#include <iostream>
#include <string>
#include <sys/time.h>

using namespace std;

class Timestamp
{
  private:
    int64_t microSecondsSinceEpoch_;

  public:
    static const int kMicroSecondsPerSecond = 1000 * 1000;
    Timestamp() : microSecondsSinceEpoch_(0)
    {
    }
    explicit Timestamp(int64_t microSecondsSinceEpoch) : microSecondsSinceEpoch_(microSecondsSinceEpoch)
    {
    }
    static Timestamp now()
    {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        int64_t seconds = tv.tv_sec;
        return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
    }
    string toFormattedString(bool showMicroseconds) const
    {
        char buf[64] = {0};
        time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
        tm *tm_time = localtime(&seconds);
        if (showMicroseconds)
        {
            int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
            snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d.%06d", tm_time->tm_year + 1900,
                     tm_time->tm_mon + 1, tm_time->tm_mday, tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec,
                     microseconds);
        }
        else
        {
            snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d", tm_time->tm_year + 1900, tm_time->tm_mon + 1,
                     tm_time->tm_mday, tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
        }
        return buf;
    }
    bool valid() const
    {
        return microSecondsSinceEpoch_ > 0;
    }
    int64_t microSecondsSinceEpoch() const
    {
        return microSecondsSinceEpoch_;
    }
    time_t secondsSinceEpoch() const
    {
        return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    }
    static Timestamp invalid()
    {
        return Timestamp();
    }

    Timestamp addTime(double seconds)
    {
        return Timestamp(microSecondsSinceEpoch_ + seconds * kMicroSecondsPerSecond);
    }
    bool operator<(const Timestamp &t) const
    {
        return microSecondsSinceEpoch_ < t.microSecondsSinceEpoch_;
    }
    bool operator==(const Timestamp &t) const
    {
        if (this == &t)
            return true;
        return microSecondsSinceEpoch_ == t.microSecondsSinceEpoch_;
    }
};