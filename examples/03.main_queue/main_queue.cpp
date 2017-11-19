
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

//=== Main Queue =============================================================

void
mainQueueCallbacks(Nan::FunctionCallbackInfo<Value> const& args) {
  std::cerr << "dispatch on thread " << threadId() << std::endl;
  dispatch(ncd::defaultWorkQueue(), [](){
    std::cerr << "work on thread " << threadId() << std::endl;
    dispatch(ncd::mainQueue(), [](){
      std::cerr << "main queue thread " << threadId() << std::endl;
    });
  }, [](){ std::cerr << "done" << std::endl; });
}

void
pinnedObject(Nan::FunctionCallbackInfo<Value> const& args) {
  Nan::HandleScope scope;
  ncd::AsyncHandle<v8::Object> pinnedObject(args[0].As<v8::Object>());
  
  dispatch(ncd::defaultWorkQueue(), [=](){
    for (unsigned i = 0; i < 10; ++i) {
      usleep(10000);
      dispatch(ncd::mainQueue(), [=](){
        Nan::HandleScope scope;
        v8::Local<v8::Object> object = pinnedObject.jsValue();
        object->Set(Nan::New("progress").ToLocalChecked(), Nan::New(i));
      });
    }
  }, [](){ std::cerr << "done" << std::endl; });
}

//=== Init ===================================================================

void Init(Local<Object> exports) {
  exports->Set(ncd::v8str("mainQueueCallbacks"), ncd::function(mainQueueCallbacks));
  exports->Set(ncd::v8str("pinnedObject"), ncd::function(pinnedObject));
}

}  // end of anonymous namespace

NCD_NODE_ADDON(Init)
