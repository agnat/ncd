#ifndef NCD_HPP_
# define NCD_HPP_

//=============================================================================
// Not Central Dispatch
//=============================================================================

# include <sstream>
# include <algorithm>
# include <deque>

# include <iostream>

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

}  // ncd

# include <ncd/function.hpp>
# include <ncd/uv.hpp>
# include <ncd/main_queue.hpp>
# include <ncd/work_queue.hpp>

# include <ncd/async_handle.hpp>
# include <ncd/async_function.hpp>

namespace ncd {

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
