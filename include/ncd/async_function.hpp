#ifndef NCD_ASYNC_FUNCTION_HPP_
# define NCD_ASYNC_FUNCTION_HPP_

namespace ncd {

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


}  // end of namespace ncd
#endif  // NCD_ASYNC_FUNCTION_HPP_
