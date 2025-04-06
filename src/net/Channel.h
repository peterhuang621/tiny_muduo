#include "Logging.h"
#include "Timestamp.h"
#include "noncopyable.h"
#include <functional>
#include <memory>

using namespace std;

class EventLoop;

class Channel : noncopyable
{
  private:
    EventLoop *loop_;
    const int fd_;
    int events_, revents_;

  public:
    using EventCallback = function<void()>;
    using ReadEventCallback = function<void(Timestamp)>;
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
    Channel(EventLoop *loop, int fd);
    ~Channel()
    {
        if (loop_->isInLoopThread())
        {
            assert(!loop_->hasChannel(this));
        }
    }
    void handleEvent(Timestamp receiveTime);
    void setReadCallback(ReadEventCallback cb)
    {
        readCallback_ = std::move(cb);
    }
    void setWriteCallback(EventCallback cb)
    {
        writeCallback_ = std::move(cb);
    }
    void setCloseCallback(EventCallback cb)
    {
        closeCallback_ = std::move(cb);
    }
    void setErrorCallback(EventCallback cb)
    {
        errorCallback_ = std::move(cb);
    }
};