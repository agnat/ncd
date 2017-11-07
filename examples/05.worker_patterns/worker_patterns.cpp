
#include <nan.h>
#include <ncd.hpp>

#include <iostream>
#include <thread>
#include <unistd.h>

namespace {

using namespace std::string_literals;
using v8::Local;
using v8::Value;
using v8::Object;

std::thread::id threadId() { return std::this_thread::get_id(); }

//============================================================================
// Inline Worker
//============================================================================

void
inlineWorker(Nan::FunctionCallbackInfo<Value> const& args) {
  unsigned delay = args[0]->Uint32Value();
  unsigned iterations = args[1]->Uint32Value();

  ncd::AsyncFunction<> progress(args[2].As<v8::Function>());
  v8::Local<v8::Function> done = args[3].As<v8::Function>();

  std::cerr << "dispatch on thread " << threadId() << std::endl;
  ncd::defaultWorkQueue().dispatch([=](){
    for (unsigned i = 0; i < iterations; ++i) {
      progress(i);
      usleep(delay);
    }
  }, done);
}

//============================================================================
// Event Emitting Worker
//============================================================================

void
eventEmittingWorker(Nan::FunctionCallbackInfo<Value> const& args) {
  unsigned delay      = args[0]->Uint32Value();
  unsigned iterations = args[1]->Uint32Value();
  ncd::AsyncEventEmitter emitter(args[2].As<v8::Object>());
  std::cerr << "dispatch on thread " << threadId() << std::endl;

  ncd::defaultWorkQueue().dispatch([=](){
    for (unsigned i = 0; i < iterations; ++i) {
      emitter.emit("progress"s, i);
      usleep(delay);
    }
  }, std::bind(emitter.emit, "done"s));
}

//============================================================================
// Worker Component
// Same as above but written as a standalone component instead of an inline
// lambda.
//============================================================================

class WorkerComponent {
public:
  WorkerComponent(unsigned delay, unsigned iterations, v8::Local<Object> eventEmitter)
    : mDelay(delay), mIterations(iterations)
    , mEmit(eventEmitter->Get(ncd::v8str("emit")).As<v8::Function>(), eventEmitter)
  {}
  void
  operator()() {
    for (unsigned i = 0; i < mIterations; ++i) {
      mEmit("progress"s, i);
      usleep(mDelay);
    }
  }
private:
  unsigned mDelay;
  unsigned mIterations;
  ncd::AsyncFunction<> mEmit;
};

void
workerComponent(Nan::FunctionCallbackInfo<Value> const& args) {
  unsigned delay = args[0]->Uint32Value();
  unsigned iterations = args[1]->Uint32Value();
  v8::Local<v8::Object> emitter = args[2].As<v8::Object>();
  v8::Local<v8::Function> emit_ = emitter->Get(Nan::New("emit").ToLocalChecked()).As<v8::Function>();

  ncd::Function<> emit(emit_, emitter);

  std::cerr << "dispatch on thread " << threadId() << std::endl;
  ncd::defaultWorkQueue().dispatch(WorkerComponent(delay, iterations, emitter),
                                   std::bind(emit, "done"s));
}

//=== Init ===================================================================

void Init(Local<Object> exports) {
  exports->Set(ncd::v8str("inlineWorker"), ncd::function(inlineWorker));
  exports->Set(ncd::v8str("eventEmittingWorker"), ncd::function(eventEmittingWorker));
  exports->Set(ncd::v8str("workerComponent"), ncd::function(workerComponent));
}

}  // end of anonymous namespace

NODE_MODULE(basic_work, Init)
