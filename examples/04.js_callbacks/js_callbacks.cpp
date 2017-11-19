
#include <nan.h>
#include <ncd.hpp>

#include <iostream>
#include <thread>
#include <unistd.h>

namespace {

using v8::Local;
using v8::Value;
using v8::Object;

std::thread::id threadId() { return std::this_thread::get_id(); }

//=== JS Done Handlers =======================================================

void
jsDoneHandlers(Nan::FunctionCallbackInfo<Value> const& args) {
  std::cerr << "dispatch on thread " << threadId() << std::endl;
  dispatch(ncd::defaultWorkQueue(), [](){}, args[0].As<v8::Function>());

  dispatch(ncd::defaultWorkQueue(), [](){ return 5.0; }, args[0].As<v8::Function>());

  dispatch(ncd::defaultWorkQueue(), [](ncd::AsyncError ** error){
    *error = new ncd::AsyncError("Kaputt");
    return 0.0;
  }, args[0].As<v8::Function>());
}

//=== Async Function =========================================================

void
asyncFunctions(Nan::FunctionCallbackInfo<Value> const& args) {
  std::cerr << "dispatch on thread " << threadId() << std::endl;
  ncd::AsyncFunction<> callback(args[0].As<v8::Function>());

  dispatch(ncd::defaultWorkQueue(), [=](){
    callback(0, 1, 2, 3, 4);
  }, args[1].As<v8::Function>());
}

//=== Init ===================================================================

void Init(Local<Object> exports) {
  exports->Set(ncd::v8str("jsDoneHandlers"), ncd::function(jsDoneHandlers));
  exports->Set(ncd::v8str("asyncFunctions"), ncd::function(asyncFunctions));
}

}  // end of anonymous namespace

NCD_NODE_ADDON(Init)
