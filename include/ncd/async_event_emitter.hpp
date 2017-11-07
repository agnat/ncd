#ifndef NCD_ASYNC_EVENT_EMITTER_HPP_
# define NCD_ASYNC_EVENT_EMITTER_HPP_

# include <ncd/async_function.hpp>

namespace ncd {

struct AsyncEventEmitter {
  AsyncEventEmitter(v8::Local<v8::Object> ee)
    : emit(ee->Get(v8str("emit")).As<v8::Function>(), ee)
  {}
  AsyncFunction<> emit;
};

}  // end of namespace ncd
#endif  // NCD_ASYNC_EVENT_EMITTER_HPP_
