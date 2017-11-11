// Copyright 2017 David Siegel. Distributed under the MIT license. See LICENSE.
#ifndef NCD_ASYNC_VALUE_HPP_
# define NCD_ASYNC_VALUE_HPP_

# include <nan.h>

# include <ncd/async_handle.hpp>
# include <ncd/converters.hpp>
# include <ncd/v8_utils.hpp>

namespace ncd {

template <typename T, typename Converters = ConvenientConverters>
class AsyncValue : AsyncHandle<typename Converters::template JSType<T>::type> {
  using V8Type = typename Converters::template JSType<T>::type;
public:
  explicit
  AsyncValue(v8::Local<v8::Value> v) 
    : AsyncHandle<V8Type>(v.As<V8Type>()), value()
  {
    mNullOrUndefined = v->IsNullOrUndefined();
    if (!instanceof<V8Type>(v)) {
      std::cerr << "Kaputt: Type error... but no proper error handling" << std::endl;
    }
    value = Converters::template extract<T, V8Type>(v);
  }

  operator bool() const { 
    return !mNullOrUndefined;
  }
  
  static
  AsyncValue<T, Converters>
  sentinel() { return AsyncValue(); }
  
  T value;

private:
  AsyncValue() : mNullOrUndefined(true) {}

  bool mNullOrUndefined;
};

}  // end of namespace ncd
#endif  // NCD_ASYNC_VALUE_HPP_
