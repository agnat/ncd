
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

//=============================================================================
// Free Functions
// A free function is the most basic form of work in ncd. It works but since
// they are stateless they are not very common.
//=============================================================================

void work() { std::cerr << "work on thread " << threadId() << std::endl; }
void done() { std::cerr << "done on thread " << threadId() << std::endl; }

void
workFunction(Nan::FunctionCallbackInfo<Value> const& args) {
  std::cerr << "dispatch on thread " << threadId() << std::endl;
  ncd::defaultWorkQueue().dispatch(work, done);
}

//=============================================================================
// Callable Object
// Since they hold state callable objects are much more usefull than free
// functions. Note how an argument is passed at construction-time. Later, the
// parameter is available on the other thread.
//=============================================================================

struct Work {
  Work(int param) : mParam(param) {
    std::cerr << "constructor on thread " << threadId() << std::endl;
  }
  void
  operator()() {
    std::cerr << "work on thread " << threadId() 
              << " with param " << mParam << std::endl;
  }
  int mParam;
};

void workObject(Nan::FunctionCallbackInfo<Value> const& args) {
  std::cerr << "dispatch on thread " << threadId() << std::endl;
  ncd::defaultWorkQueue().dispatch(Work(7), done);
}

//=============================================================================
// Lambda Expression
//=============================================================================

void workLambda(Nan::FunctionCallbackInfo<Value> const& args) {
  std::cerr << "dispatch on thread " << threadId() << std::endl;
  ncd::defaultWorkQueue().dispatch([](){
    std::cerr << "work on thread " << threadId() << std::endl;
  }, done);
}

//=== Init ===================================================================

void Init(Local<Object> exports) {
  exports->Set(ncd::v8str("workFunction"), ncd::function(workFunction));
  exports->Set(ncd::v8str("workObject"),   ncd::function(workObject));
  exports->Set(ncd::v8str("workLambda"),   ncd::function(workLambda));
}

}  // end of anonymous namespace

NCD_NODE_ADDON(Init)
