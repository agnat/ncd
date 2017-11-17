// Copyright 2017 David Siegel. Distributed under the MIT license. See LICENSE.
#ifndef NCD_V8_UTILS_HPP_
# define NCD_V8_UTILS_HPP_

# include <ncd/configuration.hpp>

# include <nan.h>

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

using v8argvec = std::vector<v8::Local<v8::Value>>;

template <typename CallbackInfo>
void
v8args2vector(CallbackInfo const& args, v8argvec & out) {
  out.clear();
  out.reserve(args.Length());
  for (int i = 0; i < args.Length(); ++i) {
    out.push_back(args[i]);
  }
}

template <typename T>
T*
beginPtr(std::vector<T> & v) { return &*v.begin(); }

}  // end of namespace ncd
#endif  // NCD_V8_UTILS_HPP_
