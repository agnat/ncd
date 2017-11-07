# ncd – not central dispatch

ncd is a library to support asynchronous C++ node addons. It provides a number of primitives to work with the UV threadpool. The library uses semantics familiar to every node developer. It allows for a very casual coding style while still facilitating the development of reusable components.

The core principle of ncd is: Pass code, not data. The user dispatches code to run on the pool or the main thread respectivley. The use is free to choose from plain functions, callable objects or lambda expressions. Anything callable will work.

## Appetizer

Let's look at a first example, the implementation of a simple worker:

````c++
void
eventEmittingWorker(Nan::FunctionCallbackInfo<Value> const& args) {
  unsigned delay      = args[0]->Uint32Value();
  unsigned iterations = args[1]->Uint32Value();

  ncd::AsyncEventEmitter emitter(args[2].As<v8::Object>());  // (1)

  ncd::defaultWorkQueue().dispatch([=](){                    // (2)
    for (unsigned i = 0; i < iterations; ++i) {
      emitter.emit("progress"s, i);                          // (3)
      usleep(delay);
    }
  }, std::bind(emitter.emit, "done"s));                      // (4)
}
````

After grabbing some arguments the code creates an `AsyncEventEmitter` in (1). It wraps a javascript `EventEmitter` for use on a different thread. In (2) a lambda expression is dipatched to the threadpool. The expression captures copies of `delay`, `iterations` and the `emitter`. This is an `ncd` pattern. `AsyncSomething` is first allocated on the main thread and then copied around to different threads. The call to `dispatch(...)` returns immediately and the lambda is launched on the thredpool. In (3) the async event emitter is invoked, sending progress events back to javascript. After the execution finishes the callback passed in (4) is invoked on the main thread. We simply emit a done event. Note how `AsyncFunction`s, the thing behind `AsyncEventEmitter` plays nicely with `std::bind(...)`.

The javascript code looks like this:

````javascript
const workers = require('workers')
  , EventEmitter = require('events')
  , ee = new EventEmitter()
    . on('progress', (p) => { console.log('progress', p) })
    . on('done',     ( ) => { console.log('done') })

workers.eventEmittingWorker(10000, 10, ee)
````

## Queues

At the heart of ncd are two types of code queues. The user dispatches code to a queue and the code is executed on the other side of a thread boundary. `WorkQueue`s run their code on the threadpool. The executing code in turn has access to an instance of `MainQueue`. Code dispatched on this type of queue executes on the main thread and hence can use the javascript engine.

These pairs of queues provide generic, bidirectional inter-thread communication.

## Handles and Functions

ncd introduces `AsyncHandle<>`s. These handles are used to keep javascript objects alive while work is executing on a thread. This solves many common housekeeping tasks in an unobtrusive way.

`AsyncFunctions` are thread-safe wrappers of javascript functions. They have the syntax of normal function calls but behind the scenes they emit an event on the main thread and invoke the function there.
