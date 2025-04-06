#pragma once

#include "Thread.h"
#include "noncopyable.h"
#include <condition_variable>
#include <deque>
#include <mutex>
#include <vector>

class ThreadPool : noncopyable
{
    using Task = function<void()>;

  private:
    void runInThread()
    {
        try
        {
            if (threadInitCallback_)
            {
                threadInitCallback_();
            }

            while (running_)
            {
                Task task;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    cond_.wait(lock, [this]() { return !queue_.empty() && running_; });
                    if (!queue_.empty())
                    {
                        task = queue_.front();
                        queue_.pop_front();
                    }
                }
                if (task)
                {
                    task();
                }
            }
        }
        catch (...)
        {
            LOG_WARN << "runInThread throw exception";
        }
    }

    mutable mutex mutex_;
    condition_variable cond_;
    string name_;
    Task threadInitCallback_;
    vector<unique_ptr<Thread>> threads_;
    deque<Task> queue_;
    atomic_bool running_;

  public:
    explicit ThreadPool(const string &name = string("ThreadPool")) : mutex_(), cond_(), name_(name), running_(false)
    {
    }
    ~ThreadPool()
    {
        if (running_)
            stop();
    }
    void setThreadInitCallback(const Task &cb)
    {
        threadInitCallback_ = cb;
    }

    void start(int numThreads)
    {
        running_ = true;
        threads_.reserve(numThreads);
        for (int i = 0; i < numThreads; ++i)
        {
            char id[32];
            snprintf(id, sizeof(id), "%d", i + 1);
            threads_.emplace_back(new Thread(std::bind(&ThreadPool::runInThread, this), name_ + id));
            threads_[i]->start();
        }
        if (numThreads == 0 && threadInitCallback_)
        {
            threadInitCallback_();
        }
    }
    void stop()
    {
        running_ = false;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.notify_all();
        }
        for (auto &thr : threads_)
            thr->join();
    }

    const string &name() const
    {
        return name_;
    }
    size_t queueSize() const
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.size();
    }

    void run(Task task)
    {
        if (threads_.empty())
        {
            task();
            return;
        }
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push_back(std::move(task));
        cond_.notify_one();
    }
};