#pragma once

#include "CurrentThread.h"
#include "Thread.h"
#include "noncopyable.h"

#include <atomic>
#include <functional>
#include <memory>
#include <semaphore>
#include <string>
#include <thread>

using namespace std;

class Thread
{
    using ThreadFunc = function<void()>;

  private:
    bool started_;
    bool joined_;
    shared_ptr<thread> thread_;
    pid_t tid_;
    ThreadFunc func_;
    string name_;
    static atomic<int32_t> numCreated_;
    void setDefaultName()
    {
        int num = (++numCreated_);
        if (name_.empty())
        {
            char buf[32] = {0};
            snprintf(buf, sizeof(buf), "Thread%d", num);
            name_ = buf;
        }
    }

  public:
    explicit Thread(ThreadFunc f, const string &name = string())
        : started_(false), joined_(false), tid_(0), func_(std::move(f)), name_(name)
    {
        setDefaultName();
    }
    ~Thread()
    {
        if (started_ & (!joined_))
            thread_->detach();
    }

    void start()
    {
        started_ = true;
        mutex mtx;
        condition_variable cv;
        bool ready = false;

        thread_ = make_shared<thread>([&]() {
            {
                unique_lock<mutex> lock(mtx);
                tid_ = CurrentThread::tid();
                ready = true;
            }
            cv.notify_one();
            func_();
        });

        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [&]() { return ready; });
    }
    void join()
    {
        joined_ = true;
        thread_->join();
    }

    bool started() const
    {
        return started_;
    }
    pid_t tid() const
    {
        return tid_;
    }
    const string &name() const
    {
        return name_;
    }
    static int numCreated()
    {
        return numCreated_;
    }
};

atomic<int32_t> Thread::numCreated_(0);
