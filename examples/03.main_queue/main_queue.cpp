
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
  ncd::defaultWorkQueue().dispatch([](){
    std::cerr << "work on thread " << threadId() << std::endl;
    ncd::mainQueue().dispatch([](){
      std::cerr << "main queue thread " << threadId() << std::endl;
    });
  }, [](){ std::cerr << "done" << std::endl; });
}

//=== Init ===================================================================

void Init(Local<Object> exports) {
  exports->Set(ncd::v8str("mainQueueCallbacks"), ncd::function(mainQueueCallbacks));
}

}  // end of anonymous namespace

NCD_NODE_ADDON(Init)
