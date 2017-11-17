// Copyright 2017 David Siegel. Distributed under the MIT license. See LICENSE.
#ifndef NCD_CONVERTERS_HPP_
# define NCD_CONVERTERS_HPP_

# include <ncd/configuration.hpp>

# include <iostream>  // XXX

# include <ncd/async_error.hpp>
# include <ncd/v8_utils.hpp>

namespace ncd {

namespace detail {

template <typename V8Type>
v8::Local<V8Type>
toLocalChecked(v8::Local<V8Type> v) { return v; }

template <typename V8Type>
v8::Local<V8Type>
toLocalChecked(v8::MaybeLocal<V8Type> v) { return v.ToLocalChecked(); }


template <typename T>
struct SelectJSType :
  std::conditional<
      std::is_arithmetic<T>::value,
      v8::Number,
      typename std::conditional<
        std::is_same<T, std::string>::value,
        v8::String,
        typename std::conditional<
          std::is_pointer<T>::value,
          v8::Object,
          v8::Value
        >::type
      >::type
  >
{};

template <typename T> struct InstanceOf;

template <>
struct InstanceOf<v8::Number> {
  static bool check(v8::Local<v8::Value> v) { return v->IsNumber(); }
};

template <>
struct InstanceOf<v8::Int32> {
  static bool check(v8::Local<v8::Value> v) { return v->IsInt32(); }
};

template <>
struct InstanceOf<v8::Uint32> {
  static bool check(v8::Local<v8::Value> v) { return v->IsUint32(); }
};

template <>
struct InstanceOf<v8::Object> {
  static bool check(v8::Local<v8::Value> v) { return v->IsObject(); }
};

template <>
struct InstanceOf<v8::Function> {
  static bool check(v8::Local<v8::Value> v) { return v->IsFunction(); }
};


template <typename T> struct Extractor;

template <typename T>
struct Extractor<T*> {
  static T* extract(v8::Local<v8::Value> v) {
    return static_cast<T*>(v.As<v8::Object>()->GetAlignedPointerFromInternalField(0)); // XXX
  }
};

template <typename T>
struct FloatExtractor {
  static T extract(v8::Local<v8::Value> v) { return v->NumberValue(); }
};

template <> struct Extractor<double> : FloatExtractor<double> {};
template <> struct Extractor<float> : FloatExtractor<float> {};

}  // end of namespace detail

template <typename V8Type>
bool
instanceof(v8::Local<v8::Value> v) { return detail::InstanceOf<V8Type>::check(v); }

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

  static
  v8::Local<v8::Value>
  toJS(char const* value) {
    return toJS(std::string(value));
  }

  template <typename T>
  using JSType = detail::SelectJSType<T>;

  template <typename T, typename V8Type>
  static
  T
  extract(v8::Local<v8::Value> value) { 
    return detail::Extractor<T>::extract(value);
  }
};

}  // end of namespace ncd
#endif  // NCD_CONVERTERS_HPP_
