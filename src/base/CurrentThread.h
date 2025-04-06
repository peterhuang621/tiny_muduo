#pragma once

#include <sys/syscall.h>
#include <unistd.h>
using namespace std;

namespace CurrentThread
{
thread_local int t_cachedTid = 0;

void cacheTid()
{
    if (t_cachedTid == 0)
        t_cachedTid = static_cast<pid_t>(syscall(SYS_gettid));
}

inline int tid()
{
    if (__builtin_expect(t_cachedTid == 0, 0))
    {
        cacheTid();
    }
    return t_cachedTid;
}
} // namespace CurrentThread
