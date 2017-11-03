#ifndef NCD_WORK_QUEUE_HPP_
# define NCD_WORK_QUEUE_HPP_

//# define NCD_TRACE_WORK_QUEUE
//# define NCD_TRACE_WORK

# ifdef NCD_TRACE_WORK_QUEUE
#  define NCD_DBWQ(x) std::cerr << x << std::endl
# else
#  define NCD_DBWQ(x)
# endif

# ifdef NCD_TRACE_WORK
#  define NCD_DBWRK(x) std::cerr << x << std::endl
# else
#  define NCD_DBWRK(x)
# endif

namespace ncd {

//=============================================================================
// Work Queue
//=============================================================================

class WorkQueue;

namespace detail {

struct MainQueueTLSKey {};
using TLSMainQueue = ThreadLocal<MainQueueTLSKey, MainQueue>;

class WorkRequest {
public:
  template <typename W, typename CB>
  WorkRequest(W & work, WorkQueue & owner, CB & done) 
    : mWorkHandle(), mMainQueue(), mOwner(owner)
    , mWorkFunction(work), mDoneFunction(done)
  {
    NCD_DBWRK("WorkRequest::WorkRequest(...)");
    mWorkHandle.data = this;
  }
  
# ifdef NCD_TRACE_WORK
  ~WorkRequest() {
    NCD_DBWRK("WorkRequest::~WorkRequest()");
  }
#endif

public:  // API
  void queue(uv_loop_t * loop) {
    NCD_DBWRK("WorkRequest::queue(...)");
    mMainQueue = std::make_unique<MainQueue>(loop);
    uv_queue_work(loop, &mWorkHandle, OnWork, OnDone);
  }

private:  // work handler
  // virtual
  void
  onWork() {
    NCD_DBWRK("WorkRequest::onWork()");
    TLSMainQueue::set(&*mMainQueue);
    mWorkFunction();
    TLSMainQueue::set(nullptr);
  };

  static
  void
  OnWork(uv_work_t * handle) {
    nauv_unwrap<WorkRequest>(handle)->onWork();
  }

private:  // done handler
  // virtual
  void
  onDone() {
    mDoneFunction();
  };

  void
  handleDone(int status);  // See below.

  static
  void
  OnDone(uv_work_t * handle, int status) {
    nauv_unwrap<WorkRequest>(handle)->handleDone(status);
  }
  
private:
  using MainQueuePtr = std::unique_ptr<MainQueue>;

  uv_work_t    mWorkHandle;
  MainQueuePtr mMainQueue;
  WorkQueue &  mOwner;

  std::function<void()> mWorkFunction;
  std::function<void()> mDoneFunction;
};

} // end of namespace detail


//=== WorkQueue ===============================================================

class WorkQueue {
private:  // types
  using WorkPtr = std::unique_ptr<detail::WorkRequest>;
  using WorkSet = std::vector<WorkPtr>;
  using Queue   = std::deque<WorkPtr>;

public:  // constructors & destructor
  WorkQueue(uv_loop_t * loop, size_t threadCount)
    : mLoop(loop), mThreadCount(threadCount)
  {
    NCD_DBWQ("WorkQueue::WorkQueue(...) threads: " << mThreadCount);
  }
# ifdef NCD_TRACE_WORK_QUEUE
  ~WorkQueue() { 
    NCD_DBWQ("WorkQueue::~WorkQueue()");
  }
# endif


  template <typename Work, typename Callback>
  void
  dispatch(Work && work, Callback && done) {
    push(work, done);
  }

  template <typename Work>
  void
  dispatch(Work && work, v8::Local<v8::Function> done) {
    dispatch(work, Function<void()>(done));
  }

protected: friend detail::WorkRequest;
  void
  doneWith(detail::WorkRequest * work) {
    NCD_DBWQ("WorkQueue::doneWith(...)");
    ScopedLock scopedLock(mQueueLock);
    WorkSet::iterator it = find_if(mCurrentWork.begin(), mCurrentWork.end(),
        [&](WorkPtr const& w) { return w.get() == work; });
    mCurrentWork.erase(it);
    scheduleWork();
  }

private:  // member functions

  template <typename Work, typename Callback>
  void
  push(Work & work, Callback & done) {
    NCD_DBWQ("WorkQueue::push(...)");
    ScopedLock scopedLock(mQueueLock);
    mPendingWork.emplace_back(std::make_unique<detail::WorkRequest>(work, *this, done));
    scheduleWork();
  }
  
  void  // callers are required to hold the queue lock
  scheduleWork() {
    while (mCurrentWork.size() < mThreadCount && ! mPendingWork.empty()) {
      WorkPtr work = std::move(mPendingWork.front());
      mPendingWork.pop_front();
      work->queue(mLoop);
      mCurrentWork.emplace_back(std::move(work));
    }
  }

private:  // data members
  uv_loop_t * mLoop;
  size_t      mThreadCount;

  Mutex   mQueueLock;
  WorkSet     mCurrentWork;
  Queue       mPendingWork;
};

# undef NCD_DBWQ

// result is valid in work functions only
inline
MainQueue&
mainQueue() {
  auto queue = detail::TLSMainQueue::get();
  assert(queue != nullptr);
  return *queue;
}

namespace detail {
bool
isMainThread() {
  return detail::TLSMainQueue::get() == nullptr;
}
}  // end of namespace detail



}  // end of namespace ncd
#endif  // NCD_WORK_QUEUE_HPP_
