// Copyright 2017 David Siegel. Distributed under the MIT license. See LICENSE.
#ifndef NCD_MAIN_QUEUE_HPP_
# define NCD_MAIN_QUEUE_HPP_

# include <ncd/configuration.hpp>

# include <deque>
# include <functional>

# include <nan.h>

# include <ncd/uv.hpp>

//# define NCD_TRACE_MAIN_QUEUE
//# define NCD_TRACE_ASYNC

# ifdef NCD_TRACE_MAIN_QUEUE
#  define NCD_DBMQ(x) std::cerr << x << std::endl
# else
#  define NCD_DBMQ(x)
# endif

# ifdef NCD_TRACE_ASYNC
#  define NCD_DBASN(x) std::cerr << x << std::endl
# else
#  define NCD_DBASN(x)
# endif

namespace ncd {


//=============================================================================
// Main Queue
//=============================================================================

namespace detail {

class AsyncRequest {
public:  // constructors & destructor
  template <typename Handler>
  explicit
  AsyncRequest(Handler && handler) : mHandler(handler) {
    NCD_DBASN("AsyncRequest::AsyncRequest()");
  }
# ifdef NCD_TRACE_ASYNC
  ~AsyncRequest() {
    NCD_DBASN("AsyncRequest::~AsyncRequest()");
  }
# endif

  void
  operator()() {
    NCD_DBASN("AsyncRequest::operator()");
    mHandler();
  }

public:  
private:
  std::function<void()> mHandler;
};

#undef NCD_DBASN

class WorkRequestBase;

}  // end of namespace detail

class MainQueue {
private:  // types
  using AsyncRequestPtr = std::unique_ptr<detail::AsyncRequest>;
public:  // constructors & destructor
  explicit MainQueue(uv_loop_t * loop)
    : mQueueLock(), mPendingItems(), mHandle(new uv_async_t())
  {
    NCD_DBMQ("MainQueue::MainQueue(...)");
    mHandle->data = this;
    uv_async_init(loop, mHandle, OnFlush);
  }
  ~MainQueue() {
    NCD_DBMQ("MainQueue::~MainQueue()");
    uv_close((uv_handle_t*)mHandle, OnClose);
  }

public:  // API
  template <typename Callback>
  void
  dispatch(Callback && callback) {
    push(callback);
  }

private: friend detail::WorkRequestBase;
  void
  flush() {
    Nan::HandleScope scope;
    ScopedLock scopedLock(mQueueLock);
    while (!mPendingItems.empty()) {
      AsyncRequestPtr requestPtr = std::move(mPendingItems.front());
      mPendingItems.pop_front();
      detail::AsyncRequest & request = *requestPtr;
      scopedLock.unlock();
      request();
      scopedLock.lock();
    }
  }
  
private:  // event handlers
  static
  void
  OnFlush(uv_async_t * handle) {
    nauv_unwrap<MainQueue>(handle)->flush();
  }

  static
  void
  OnClose(uv_handle_t * handle) { delete handle; }
  
private:  // member functions
  template <typename Callback>
  void
  push(Callback && callback) {
    NCD_DBMQ("MainQueue::push(...)");
    ScopedLock scopedLock(mQueueLock);
    mPendingItems.emplace_back(std::make_unique<detail::AsyncRequest>(callback));
    uv_async_send(mHandle);
  }

private:  // data members
  using QueueType = std::deque<AsyncRequestPtr>;
  Mutex    mQueueLock;
  QueueType    mPendingItems;
  uv_async_t * mHandle;
};

# undef NCD_DBMQ

}  // end of namespace ncd
#endif  // NCD_MAIN_QUEUE_HPP_
