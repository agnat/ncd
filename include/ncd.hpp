#ifndef NCD_HPP_
# define NCD_HPP_

//=============================================================================
// Not Central Dispatch
//=============================================================================

# include <sstream>
# include <algorithm>
# include <deque>

# include <iostream>

//# include <ncd/meta.hpp>

//# define NCD_TRACE_WORK_QUEUE
//# define NCD_TRACE_WORK
//# define NCD_TRACE_MAIN_QUEUE
//# define NCD_TRACE_ASYNC
//# define NCD_TRACE_HANDLES

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

# ifdef NCD_TRACE_HANDLES
#  include <thread>
#  define NCD_HANDLE_TRACER_ARG(T) , detail::TracingDeleter<T>()
#  define NCD_DBHD(x) std::cerr << x << std::endl
# else
#  define NCD_HANDLE_TRACER_ARG(T)
#  define NCD_DBHD(x)
# endif

namespace ncd {

inline
v8::Local<v8::String>
v8str(std::string const& str) {
  return Nan::New(str).ToLocalChecked();
}

inline
v8::Local<v8::Function>
function(Nan::FunctionCallback f) {
  return Nan::New<v8::Function>(f);
}

namespace detail {
template <typename T>
using CopyablePersistent = v8::Persistent<T, v8::CopyablePersistentTraits<T>>;
} // end of namespace detail

//=============================================================================
// Function
//=============================================================================

namespace detail {

template <typename V8Type>
v8::Local<V8Type>
toLocalChecked(v8::Local<V8Type> v) { return v; }

template <typename V8Type>
v8::Local<V8Type>
toLocalChecked(v8::MaybeLocal<V8Type> v) { return v.ToLocalChecked(); }

}  // end of namespace detail

struct ConvenientConverters {
  template <typename T>
  static
  v8::Local<v8::Value>
  toJS(T const& value) {
    return detail::toLocalChecked(Nan::New(value));
  }
};


template <typename Converters>
class FunctionBase {
public:
  explicit
  FunctionBase(v8::Local<v8::Function> f, v8::Local<v8::Value> target)
    : mFunction(v8::Isolate::GetCurrent(), f), mTarget(v8::Isolate::GetCurrent(), target)
  {}
  explicit
  FunctionBase(v8::Local<v8::Function> f)
    : mFunction(v8::Isolate::GetCurrent(), f), mTarget()
  {}

  using ReturnType = v8::Local<v8::Value>;

protected:
  template <typename... Args>
  ReturnType
  call(Args const&... args) const {
    Nan::HandleScope handleScope;
    const size_t argc = sizeof...(args);
    v8::Local<v8::Value> argv[argc] = { Converters::toJS(args)... };
    v8::Local<v8::Value> target = Nan::New(mTarget);
    if (target.IsEmpty()) {
      target = Nan::Undefined();
    }
    return Nan::New(mFunction)->Call(target, argc, argv);
  }

private:
  detail::CopyablePersistent<v8::Function> mFunction;
  detail::CopyablePersistent<v8::Value>   mTarget;
};

template <typename Backend, typename... T> class FunctionAPI;

template <typename Backend, typename R, typename... Args>
class FunctionAPI<Backend, R(Args...)> : public Backend {
public:
  using Backend::Backend;

  R
  operator()(Args const&... args) const {
    this->call(args...);
    return R(); // XXX
  }
};

template <typename Backend, typename R>
class FunctionAPI<Backend, R> : public Backend {
public:
  using Backend::Backend;

  template <typename... Args>
  R
  operator()(Args const&... args) {
    this->call(args...);
    return R(); // XXX
  }
};

template <typename Backend>
class FunctionAPI<Backend> : public Backend {
public:
  using Backend::Backend;

  template <typename... Args>
  typename Backend::ReturnType
  operator()(Args const&... args) const {
    return this->call(args...);
  }
};

template <typename Converters, typename... T>
using FunctionWithConverters = FunctionAPI<FunctionBase<Converters>, T...>;

template <typename... T>
using Function = FunctionWithConverters<ConvenientConverters, T...>;

//=============================================================================
// NAN UV RAII Wrappers
//=============================================================================


class Mutex {
public:
  Mutex() { uv_mutex_init(&mMutex); }
  ~Mutex() { uv_mutex_destroy(&mMutex); }

  void lock() { uv_mutex_lock(&mMutex); }
  void unlock() { uv_mutex_unlock(&mMutex); }

private:
  friend class Condition;
  uv_mutex_t mMutex;
};

class ScopedLock {
public:
  ScopedLock(Mutex & mutex) : mMutex(mutex) { mMutex.lock(); }
  ~ScopedLock() { mMutex.unlock(); }

  void lock() { mMutex.lock(); }
  void unlock() { mMutex.unlock(); }

private:
  Mutex & mMutex;
};


class Condition {
public:
  Condition()  { uv_cond_init(&mCondition); }
  ~Condition() { uv_cond_destroy(&mCondition); }

  void wait(Mutex & mutex) { uv_cond_wait(&mCondition, &mutex.mMutex); }
  void signal() { uv_cond_signal(&mCondition); }
  void broadcast() { uv_cond_broadcast(&mCondition); }

private:
  uv_cond_t mCondition;
};

template <typename T, typename UVHandle>
T*
nauv_unwrap(UVHandle * handle) {
  return static_cast<T*>(handle->data);
}

template <typename Key, typename T>
class ThreadLocal {
public:
  using ValueType = T;

  static
  void
  set(ValueType * value) {
    init();
    uv_key_set(&sKey, value);
  }

  static
  ValueType*
  get() {
    init();
    return static_cast<ValueType*>(uv_key_get(&sKey));
  }

  static
  void
  init() {
    if (!sInitialized) {
      uv_key_create(&sKey);
      sInitialized = true;
    }
  }
private:
  static bool sInitialized;
  static uv_key_t sKey;
};

template <typename Key, typename T>
bool
ThreadLocal<Key,T>::sInitialized = false;

template <typename Key, typename T>
uv_key_t
ThreadLocal<Key,T>::sKey = uv_key_t();



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

class WorkRequest;

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
    uv_async_init(loop, mHandle, OnWake);
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

private: friend detail::WorkRequest;
  void
  wake() {
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
  OnWake(uv_async_t * handle) {
    nauv_unwrap<MainQueue>(handle)->wake();
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

//=== Default Queue ===========================================================

WorkQueue&
defaultWorkQueue() {
  // TODO: This should be an actual singleton...
  static std::unique_ptr<WorkQueue> sWorkQueue = nullptr;
  if (sWorkQueue == nullptr) {
    const size_t defaultThreadCount = 4;
    size_t threadCount = defaultThreadCount;
    if (char const* threadEnv = std::getenv("UV_THREADPOOL_SIZE")) {
      std::stringstream stream(threadEnv);
      int value;
      stream >> value;
      if (!stream) {
        std::cerr << "Failed to parse UV_THREADPOOL_SIZE: '"
                  << threadEnv << "'" << std::endl;
      } else {
        threadCount = value;
      }
    }
    threadCount = std::max(1ul, threadCount - defaultThreadCount);
    sWorkQueue = std::make_unique<WorkQueue>(uv_default_loop(), threadCount);
  }
  return *sWorkQueue;
}

//=== Main Queue ==============================================================

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

//=============================================================================
// Async Handles
//=============================================================================

# ifdef NCD_TRACE_HANDLES
namespace detail {
template <typename V8Type>
struct TracingDeleter {
  void
  operator()(Nan::Persistent<V8Type> * p) {
    std::cerr << "DELETED " << std::this_thread::get_id() << std::endl;
    delete p;
  }
};
}  // end of namespace detail
# endif

template <typename V8Type>
class AsyncHandle {
  using PersistentType = Nan::Persistent<V8Type>;
  using HandlePtr = std::shared_ptr<PersistentType>;
public:
  AsyncHandle() : mHandle() {
    NCD_DBHD("AsyncHandle::AsyncHandle()");
  }

  explicit
  AsyncHandle(v8::Local<V8Type> jsValue) : mHandle(wrap(jsValue)) {
    NCD_DBHD("AsyncHandle::AsyncHandle(...)");
  }
# if 0
  ~AsyncHandle() {
    NCD_DBHD("AsyncHandle::~AsyncHandle() " << std::this_thread::get_id());
    if (!detail::isMainThread()) {
      std::cerr << "QUEUED " << std::this_thread::get_id() << std::endl;
      HandlePtr copy = mHandle;
      mainQueue().dispatch([=]() mutable { copy.reset(); });
    }
  }
# endif

public:
  v8::Local<V8Type>
  jsValue() const {
    if (mHandle) {
      return Nan::New(*mHandle);
    }
    return v8::Local<V8Type>();
  }
private:
  HandlePtr
  wrap(v8::Local<V8Type> handle) {
    if (handle.IsEmpty()) {
      return nullptr;
    } else {
      return HandlePtr(new PersistentType(handle) NCD_HANDLE_TRACER_ARG(V8Type)); 
    }
  }

  HandlePtr mHandle;
};

# undef APPEND_HANDLE_TRACER
# undef NCD_DBHD


//=============================================================================
// Async Functions
//=============================================================================

template <typename Converters>
class AsyncFunctionBase {
  using FunctionType = FunctionWithConverters<Converters>;
public:
  explicit
  AsyncFunctionBase(v8::Local<v8::Function> f)
    : mFunctionHandle(std::make_shared<FunctionType>(f))
  {}
  AsyncFunctionBase(v8::Local<v8::Function> f, v8::Local<v8::Value> target)
    : mFunctionHandle(std::make_shared<FunctionType>(f, target))
  {}

  using ReturnType = void;

protected:
  template <typename... Args>
  ReturnType
  call(Args const&... args) const {
    if (detail::isMainThread()) {
      (*mFunctionHandle)(args...);
    } else {
      auto f = mFunctionHandle;
      mainQueue().dispatch([=](){ (*f)(args...); });
    }
  }
private:
  std::shared_ptr<FunctionType> mFunctionHandle;
};

template <typename Converters, typename... T>
using AsyncFunctionWithConverters = FunctionAPI<AsyncFunctionBase<Converters>, T...>;

template <typename... T>
using AsyncFunction = AsyncFunctionWithConverters<ConvenientConverters, T...>;

AsyncFunction<>
wrapEventEmitter(v8::Local<v8::Value> emitter_) {
  if (!emitter_->IsObject()) {
    std::cerr << "wrapEventEmitter() argument is not an object" << std::endl;
  }
  auto emitter = emitter_.As<v8::Object>();
  return AsyncFunction<>(emitter->Get(v8str("emit")).As<v8::Function>(), emitter);
}

using ArgumentVector = std::vector<v8::Local<v8::Value>>;

template <typename CallbackInfo>
void
v8args2vector(CallbackInfo const& args, ArgumentVector & out) {
  out.clear();
  out.reserve(args.Length());
  for (int i = 0; i < args.Length(); ++i) {
    out.push_back(args[i]);
  }
}

//=============================================================================
// Streams
//=============================================================================

template <typename T>
class StreamBase : Nan::ObjectWrap {
public:
protected:
  using QueueType = std::deque<T>;
  Mutex mMutex;
  Condition mCondition;
  QueueType mQueue;
private:
  size_t mCapacity;
};

template <typename T, typename Converters>
class WritableStream {
public:
protected:
private:
};

//=== Implementations =========================================================

namespace detail {

inline
void
WorkRequest::handleDone(int status) {
  NCD_DBWRK("WorkRequest::handleDone() status: " << status);
  mMainQueue->wake();
  onDone();
  mOwner.doneWith(this);
}

# undef NCD_DBWRK

}  // end of namespace detail

}  // end of namespace ncd
#endif  // NCD_HPP_
