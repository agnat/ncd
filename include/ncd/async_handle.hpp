#ifndef NCD_ASYNC_HANDLE_H_
# define NCD_ASYNC_HANDLE_H_

//# define NCD_TRACE_HANDLES

# ifdef NCD_TRACE_HANDLES
#  include <thread>
#  define NCD_HANDLE_TRACER_ARG(T) , detail::TracingDeleter<T>()
#  define NCD_DBHD(x) std::cerr << x << std::endl
# else
#  define NCD_HANDLE_TRACER_ARG(T)
#  define NCD_DBHD(x)
# endif

namespace ncd {

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



}  // end of namespace ncd
#endif  // NCD_ASYNC_HANDLE_H_
