#include <nan.h>
#include <ncd.hpp>

#include <unistd.h>

namespace {

void
testWorkQueue(Nan::FunctionCallbackInfo<v8::Value> const& args) {
  Nan::HandleScope scope;
  unsigned items = args[0]->Uint32Value();
  unsigned delay = args[1]->Uint32Value();
  
  for (unsigned i = 0; i < items; ++i) {
    dispatch(ncd::defaultWorkQueue(), [=](){
      if (delay != 0) {
        usleep(delay * 1000);
      }
    }, args[2].As<v8::Function>());
  }
}

void
testStringResult(Nan::FunctionCallbackInfo<v8::Value> const& args) {
  dispatch(ncd::defaultWorkQueue(),
      [=](){ return "This is fine."; },
      args[0].As<v8::Function>());
}

void
testDoubleResult(Nan::FunctionCallbackInfo<v8::Value> const& args) {
  dispatch(ncd::defaultWorkQueue(),
      [=](){ return 0.5; },
      args[0].As<v8::Function>());
}

void
testStringOrError(Nan::FunctionCallbackInfo<v8::Value> const& args) {
  bool succeed = args[0]->ToBoolean()->Value();
  dispatch(ncd::defaultWorkQueue(), [=](ncd::AsyncError ** error) {
    if (succeed) {
      return "This is fine.";
    } else {
      *error = new ncd::AsyncError("Kaputt.");
      return (char const*)nullptr;
    }
  }, args[1].As<v8::Function>());
}

void
init(v8::Local<v8::Object> exports) {
  exports->Set(ncd::v8str("testWorkQueue"), ncd::function(testWorkQueue));
  exports->Set(ncd::v8str("testStringResult"), ncd::function(testStringResult));
  exports->Set(ncd::v8str("testDoubleResult"), ncd::function(testDoubleResult));
  exports->Set(ncd::v8str("testStringOrError"), ncd::function(testStringOrError));
}

}  // end of anonymous namespace

NCD_NODE_ADDON(init)
