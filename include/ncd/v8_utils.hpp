#ifndef NCD_V8_UTILS_HPP_
# define NCD_V8_UTILS_HPP_

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

template <typename T>
T*
beginPtr(std::vector<T> const& v) { return &*v.begin(); }

}  // end of namespace ncd
#endif  // NCD_V8_UTILS_HPP_
