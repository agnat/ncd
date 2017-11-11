
#include <nan.h>
#include <ncd.hpp>

#include <iostream>
#include <thread>
#include <unistd.h>

namespace {

using v8::Local;
using v8::Value;
using v8::Object;

//std::thread::id threadId() { return std::this_thread::get_id(); }

using WriteStream = ncd::WritableStream<double>;

void
makeWriteStream(Nan::FunctionCallbackInfo<Value> const& args) {
  Nan::HandleScope scope;
  v8::Local<v8::Object> ws = WriteStream::New(args[0].As<v8::Object>());
  //ws.once("data", [](Nan::FunctionCallbackInfo<v8::Value> const& args) {});

  WriteStream::AsyncStream stream_(ws);
  ncd::defaultWorkQueue().dispatch([=](){
    std::cerr << "reader " << std::endl;
    WriteStream::AsyncEndpoint stream(stream_);
    while (ncd::AsyncValue<double> v = stream.readSync()) {
      std::cerr << "read " << v.value << std::endl;
    }
  }, [](){std::cerr << "== done ==" << std::endl;});

  args.GetReturnValue().Set(ws);
}

void
secondExport(Nan::FunctionCallbackInfo<Value> const& args) {
  Nan::HandleScope scope;
  v8::Local<v8::Object> exports(args[0].As<v8::Object>());
  v8::Local<v8::Object> util(args[1].As<v8::Object>());
  v8::Local<v8::Object> stream(args[2].As<v8::Object>());

  v8::Local<v8::Function> Writable = stream->Get(ncd::v8str("Writable")).As<v8::Function>();
  WriteStream::Constructor("WriteStream", Writable, util);
  exports->Set(ncd::v8str("makeWriteStream"), ncd::function(makeWriteStream));
}

void
Init(Local<Object> exports) {
  Nan::HandleScope scope;
  exports->Set(ncd::v8str("secondExport"), ncd::function(secondExport));
}

}  // end of anonymous namespace

NCD_NODE_ADDON(Init)
