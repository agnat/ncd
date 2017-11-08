// Copyright 2017 David Siegel. Distributed under the MIT license. See LICENSE.
#ifndef NCD_CONVERTERS_HPP_
# define NCD_CONVERTERS_HPP_

# include <ncd/async_error.hpp>

namespace ncd {

namespace detail {

template <typename V8Type>
v8::Local<V8Type>
toLocalChecked(v8::Local<V8Type> v) { return v; }

template <typename V8Type>
v8::Local<V8Type>
toLocalChecked(v8::MaybeLocal<V8Type> v) { return v.ToLocalChecked(); }

}  // end of namespace detail

struct ConvenientConverters {
  static
  v8::Local<v8::Value>
  toJS(AsyncError * error) {
    v8::Isolate * iso = v8::Isolate::GetCurrent();
    if (error) {
      v8::Local<v8::String> message = v8str(error->message());
      delete error;
      return v8::Exception::Error(message);
    } else {
      return v8::Null(iso);
    }
  }
  template <typename T>
  static
  v8::Local<v8::Value>
  toJS(T const& value) {
    return detail::toLocalChecked(Nan::New(value));
  }
};

}  // end of namespace ncd
#endif  // NCD_CONVERTERS_HPP_
