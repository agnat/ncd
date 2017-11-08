// Copyright 2017 David Siegel. Distributed under the MIT license. See LICENSE.
#ifndef NCD_FUNCTION_HPP_
# define NCD_FUNCTION_HPP_

# include <nan.h>

# include <ncd/v8_utils.hpp>
# include <ncd/async_error.hpp>
# include <ncd/converters.hpp>

namespace ncd {

//=============================================================================
// Function
//=============================================================================


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

}  // end of namespace ncd
#endif  // NCD_FUNCTION_HPP_
