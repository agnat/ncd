
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

void work() { std::cerr << "work on thread " << threadId() << std::endl; }
void done() { std::cerr << "done on thread " << threadId() << std::endl; }

void
workFunction(Nan::FunctionCallbackInfo<Value> const& args) {
  std::cerr << "dispatch on thread " << threadId() << std::endl;
  ncd::defaultWorkQueue().dispatch(work, done);
}

struct Work {
  Work(unsigned delay_) 
    : delay(delay_)
  { std::cerr << "constructor on thread " << threadId() << std::endl; }

  void
  operator()() {
    std::cerr << "work on thread " << threadId() << std::endl; 
    usleep(delay);
  }
  unsigned delay;
};

void
workObject(Nan::FunctionCallbackInfo<Value> const& args) {
  std::cerr << "dispatch on thread " << threadId() << std::endl;
  ncd::defaultWorkQueue().dispatch(Work(1000), done);
}

void
Init(Local<Object> exports) {
  exports->Set(ncd::v8str("workFunction"), ncd::function(workFunction));
  exports->Set(ncd::v8str("workObject"), ncd::function(workObject));
}

}  // end of anonymous namespace

NODE_MODULE(basic_work, Init)
