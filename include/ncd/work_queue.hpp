#ifndef NCD_WORK_QUEUE_HPP_
# define NCD_WORK_QUEUE_HPP_

# include <ncd/meta.hpp>
# include <ncd/uv.hpp>
# include <ncd/main_queue.hpp>
# include <ncd/queue_getters.hpp>
# include <ncd/function.hpp>

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

//=== WorkRequestBase =========================================================

class WorkRequestBase {
public:
  WorkRequestBase(WorkQueue & owner) 
    : mWorkHandle(), mMainQueue(), mOwner(owner)
  {
    NCD_DBWRK("WorkRequestBase::WorkRequestBase(...)");
    mWorkHandle.data = this;
  }
  
  virtual
  ~WorkRequestBase() {
    NCD_DBWRK("WorkRequestBase::~WorkRequestBase()");
  }

public:  // API
  void
  queue(uv_loop_t * loop) {
    NCD_DBWRK("WorkRequestBase::queue(...)");
    mMainQueue = std::make_unique<MainQueue>(loop);
    uv_queue_work(loop, &mWorkHandle, OnWork, OnDone);
  }

private:  // work handler
  virtual void work() = 0;

  void
  onWork() {
    NCD_DBWRK("WorkRequestBase::onWork()");
    assert(TLSMainQueue::get() == nullptr);
    TLSMainQueue::set(&*mMainQueue);
    work();
    TLSMainQueue::set(nullptr);
  };

  static
  void
  OnWork(uv_work_t * handle) {
    nauv_unwrap<WorkRequestBase>(handle)->onWork();
  }

private:  // done handler
  virtual void done() = 0;

  void
  onDone() {
    done();
  };

  void
  handleDone(int status);  // See below.

  static
  void
  OnDone(uv_work_t * handle, int status) {
    nauv_unwrap<WorkRequestBase>(handle)->handleDone(status);
  }
  
private:  // data members
  using MainQueuePtr = std::unique_ptr<MainQueue>;

  uv_work_t    mWorkHandle;
  MainQueuePtr mMainQueue;
  WorkQueue &  mOwner;
};

//=== WorkResultHandler =======================================================

template <typename Sig, typename ReturnPolicy>
struct WorkResultHandler;

template <typename ReturnPolicy>
struct WorkResultHandler<void(), ReturnPolicy> {
  using CallbackSig = void();

  template <typename W>
  void
  invokeWork(W & work) { work(); }
  
  template <typename CB>
  void
  invokeDone(CB & done) { done(); }
};

template <typename ReturnPolicy>
struct WorkResultHandler<typename ReturnPolicy::ErrorType*(), ReturnPolicy> {
  using CallbackSig = void(typename ReturnPolicy::ErrorType*);
 
  template <typename W>
  void
  invokeWork(W & work) { mError = work(); }

  template <typename CB>
  void
  invokeDone(CB & done) { done(mError); }

  typename ReturnPolicy::ErrorType* mError;
};

template <typename ReturnPolicy>
struct WorkResultHandler<void(typename ReturnPolicy::ErrorType**), ReturnPolicy> {
  using CallbackSig = void(typename ReturnPolicy::ErrorType*);

  WorkResultHandler() : mError() {}
  
  template <typename W>
  void
  invokeWork(W & work) { work(&mError); }

  template <typename CB>
  void
  invokeDone(CB & done) { done(mError); }

  typename ReturnPolicy::ErrorType* mError;
};

template <typename R, typename ReturnPolicy>
struct WorkResultHandler<R(), ReturnPolicy> {
  using CallbackSig = void(typename ReturnPolicy::ErrorType*, R const&);

  template <typename W>
  void
  invokeWork(W & work) { mResult = work(); }

  template <typename CB>
  void
  invokeDone(CB & done) { done(nullptr, mResult); }

  R mResult;
};

template <typename R, typename ReturnPolicy>
struct WorkResultHandler<R(typename ReturnPolicy::ErrorType**), ReturnPolicy> {
  using CallbackSig = void(typename ReturnPolicy::ErrorType*, R const&);

  WorkResultHandler() : mError() {}

  template <typename W>
  void
  invokeWork(W & work) { mResult = work(&mError); }

  template <typename CB>
  void
  invokeDone(CB & done) { done(mError, mResult); }

  R mResult;
  typename ReturnPolicy::ErrorType * mError;
};

//=== WorkRequest =============================================================

template <typename WorkSig, typename ReturnPolicy>
class WorkRequest : public WorkRequestBase
                  , public WorkResultHandler<WorkSig, ReturnPolicy>
{
  using ErrorType = typename ReturnPolicy::ErrorType;
  using ResultConverter = typename ReturnPolicy::template ResultConverter<WorkSig>;
  using DoneSignature = typename WorkResultHandler<WorkSig, ReturnPolicy>::CallbackSig;

public:
  template <typename W, typename CB>
  WorkRequest(W && work, WorkQueue & owner, CB && done) 
    : WorkRequestBase(owner), mWork(work), mDone(done)
  {}

  template <typename W>
  WorkRequest(W && work, WorkQueue & owner, v8::Local<v8::Function> & done) 
    : WorkRequestBase(owner), mWork(work), mDone(Function<DoneSignature>(done))
  {}

private:
  void
  work() override { this->invokeWork(mWork); }

  void
  done() override { this->invokeDone(mDone); }

private:
  std::function<WorkSig>     mWork;
  std::function<DoneSignature> mDone; 
};

} // end of namespace detail

//=== WorkQueue ===============================================================

template <typename Error>
struct ReturnPolicy {
  using ErrorType = Error;
  using ErrorPtr = ErrorType*;

  template <typename Sig>
  struct ResultConverter;
  
  template <typename R>
  struct ResultConverter<R(ErrorPtr*)> {
    ErrorPtr error;
  };

  template <typename R>
  struct ResultConverter<R()> {
  };
};


using DefaultReturnPolicy = ReturnPolicy<AsyncError>;

class WorkQueue {
private:  // types
  using WorkPtr = std::unique_ptr<detail::WorkRequestBase>;
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
    push(work, done);
  }

protected: friend detail::WorkRequestBase;
  void
  doneWith(detail::WorkRequestBase * work) {
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
  push(Work && work, Callback && done) {
    NCD_DBWQ("WorkQueue::push(...)");
    ScopedLock scopedLock(mQueueLock);
    using Request = detail::WorkRequest<
      typename detail::callable_signature<Work>::type,
      DefaultReturnPolicy
    >;
    mPendingWork.emplace_back(std::make_unique<Request>(work, *this, done));
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

  Mutex       mQueueLock;
  WorkSet     mCurrentWork;
  Queue       mPendingWork;
};

# undef NCD_DBWQ

//=== Implementations =========================================================

namespace detail {

inline
void
WorkRequestBase::handleDone(int status) {
  NCD_DBWRK("WorkRequestBase::handleDone() status: " << status);
  mMainQueue->flush();
  onDone();
  mOwner.doneWith(this);
}

# undef NCD_DBWRK

}  // end of namespace detail


}  // end of namespace ncd
#endif  // NCD_WORK_QUEUE_HPP_
