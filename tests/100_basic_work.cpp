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
    ncd::defaultWorkQueue().dispatch([=](){
      usleep(delay * 1000);
    }, args[2].As<v8::Function>());
  }
  
}

void
init(v8::Local<v8::Object> exports) {
  exports->Set(ncd::v8str("testWorkQueue"), ncd::function(testWorkQueue));
}

}  // end of anonymous namespace

NCD_NODE_ADDON(init)
