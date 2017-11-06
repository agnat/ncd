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

}  // end of namespace ncd
#endif  // NCD_V8_UTILS_HPP_
