
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

//=== Error Handling =========================================================

ncd::AsyncError*
work_return_ok() {
  std::cerr << "success on thread " << threadId() << std::endl;
  return nullptr;
}

ncd::AsyncError*
work_return_error() {
  std::cerr << "error on thread " << threadId() << std::endl;
  return new ncd::AsyncError();
}

void
work_outparam_ok(ncd::AsyncError ** error) {
  std::cerr << "success on thread " << threadId() << std::endl;
}

void
work_outparam_error(ncd::AsyncError ** error) {
  std::cerr << "error on thread " << threadId() << std::endl;
  *error = new ncd::AsyncError();
}

void
done(ncd::AsyncError * error) {
  std::cerr << "done: " << (error ? "failed" : "ok") << std::endl;
  if (error) delete error;
}

//=== Return Values ==========================================================

double
doubleWork() { return 23.0; }

double
doubleFailedWork(ncd::AsyncError** error) {
  *error = new ncd::AsyncError();
  return 0;
}

void
done_double(ncd::AsyncError* error, double const& result) {
  if (error) {
    std::cerr << "done error: " << error << std::endl;
    delete error;
  } else {
    std::cerr << "done ok result: " << result << std::endl;
  }
}

void
workWithError(Nan::FunctionCallbackInfo<Value> const& args) {
  std::cerr << "dispatch on thread " << threadId() << std::endl;
  ncd::defaultWorkQueue().dispatch(work_return_ok, done);
  ncd::defaultWorkQueue().dispatch(work_return_error, done);
  ncd::defaultWorkQueue().dispatch(work_outparam_ok, done);
  ncd::defaultWorkQueue().dispatch(work_outparam_error, done);
  
  ncd::defaultWorkQueue().dispatch([](){ return new ncd::AsyncError();}, done);

  ncd::defaultWorkQueue().dispatch(doubleWork, done_double);
  ncd::defaultWorkQueue().dispatch(doubleFailedWork, done_double);
}

//=== Init ===================================================================

void Init(Local<Object> exports) {
  exports->Set(ncd::v8str("workWithError"), ncd::function(workWithError));
}

}  // end of anonymous namespace

NODE_MODULE(basic_work, Init)
