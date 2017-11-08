// Copyright 2017 David Siegel. Distributed under the MIT license. See LICENSE.
#ifndef NCD_JS_CLASS_HPP_
# define NCD_JS_CLASS_HPP_

# include <functional>

# include <nan.h>

# include <ncd/v8_utils.hpp>

namespace ncd {


namespace detail {

template <typename T, typename F>
struct Invoker {
  using Self = Invoker<T, F>;
  Invoker(F const& f) : memberFunction(f) {}
  static
  void
  invoke(Nan::FunctionCallbackInfo<v8::Value> const& args) {
    Self * self = static_cast<Self*>(args.Data().As<v8::External>()->Value());
    T * obj = static_cast<T*>(args.This()->GetAlignedPointerFromInternalField(0));
    self->memberFunction(*obj, args);
  }
  F memberFunction;
};

template <typename T, typename MemFn>
v8::Local<v8::FunctionTemplate>
wrapMemberFunction(MemFn const& f) {
  using invoker = Invoker<T, MemFn>;
  auto i = new invoker(f);
  return Nan::New<v8::FunctionTemplate>(invoker::invoke, Nan::New<v8::External>(i));
}

} // end of namespace detail

template <typename Derived>
class JSWrapped : public Nan::ObjectWrap {
public:
  static
  v8::Local<v8::Function>
  Constructor(std::string const& className) {
    if (sTemplate.IsEmpty()) {
      v8::Local<v8::FunctionTemplate> tpl =
        Nan::New<v8::FunctionTemplate>(Construct);
      tpl->SetClassName(v8str(className));
      tpl->InstanceTemplate()->SetInternalFieldCount(1);
      sTemplate.Reset(tpl);
      Derived::makeTemplate(tpl);
      v8::Local<v8::Function> constructor = tpl->GetFunction();
      return constructor;
    } else {
      return Nan::New(sTemplate)->GetFunction();
    }
  }

  static
  v8::Local<v8::Function>
  Constructor(std::string const& className, 
              v8::Local<v8::Function> superConstructor, 
              v8::Local<v8::Object> util) 
  {
    v8::Local<v8::Function> constructor = Constructor(className);
    sSuperConstructor.Reset(superConstructor);
    v8::Local<v8::Function> inherits = util->Get(v8str("inherits")).As<v8::Function>();
    v8::Local<v8::Value> argv[2] = { constructor, superConstructor };
    inherits->Call(Nan::Undefined(), 2, argv);
    return constructor;
  }

  template <typename... Args>
  static
  v8::Local<v8::Object>
  New(Args... args) {
    const size_t argc = sizeof...(args);
    v8::Local<v8::Value> argv[argc] = {args...};
    return Nan::New(sTemplate)->GetFunction()->NewInstance(
        v8::Isolate::GetCurrent()->GetCurrentContext(), argc, argv).ToLocalChecked();
  }

  static
  void
  makeTemplate(v8::Local<v8::FunctionTemplate>) {}


  static
  void
  prototypeMethod(std::string const& name, 
                  void(Derived::*memFn)(Nan::FunctionCallbackInfo<v8::Value> const&))
  {
    Nan::New(sTemplate)->PrototypeTemplate()->Set(v8str(name),
        detail::wrapMemberFunction<Derived>(std::mem_fn(memFn)));
  }

private:
  static
  void
  Construct(Nan::FunctionCallbackInfo<v8::Value> const& args) {
    if (args.IsConstructCall()) {
      if (!sSuperConstructor.IsEmpty()) {
        v8argvec argv;
        v8args2vector(args, argv);
        v8::Local<v8::Function> super(Nan::New(sSuperConstructor));
        super->Call(args.This(), argv.size(), beginPtr(argv));
      }
      (new Derived(args))->Wrap(args.This());
      args.GetReturnValue().Set(args.This());
    } else {
      v8argvec argv;
      v8args2vector(args, argv);
      v8::Local<v8::Function> ctor = Nan::New(sTemplate)->GetFunction();
      args.GetReturnValue().Set(ctor->NewInstance(args.GetIsolate()->GetCurrentContext(),
                                argv.size(), beginPtr(argv)).ToLocalChecked());
    }
  }
private:
  static Nan::Persistent<v8::FunctionTemplate> sTemplate;
  static Nan::Persistent<v8::Function> sSuperConstructor;
};

template <typename Derived>
Nan::Persistent<v8::FunctionTemplate>
JSWrapped<Derived>::sTemplate {};

template <typename Derived>
Nan::Persistent<v8::Function>
JSWrapped<Derived>::sSuperConstructor {};

}  // end of namespace ncd
#endif  // NCD_JS_CLASS_HPP_
