**<p align="center">Early draft – Not ready for production</p>**

# ncd – not central dispatch

ncd is a library to support asynchronous C++ node addons. It provides a number of primitives to work with the UV threadpool. The library uses semantics familiar to every node developer. It allows for a very casual coding style while still facilitating the development of reusable components.

The core principle of ncd is: Pass code, not data. The user dispatches code to run on the pool or the main thread respectivley. The user can choose from plain functions, callable objects or lambda expressions. Anything callable will work.

## Appetizer

Let's take a look at a first example, the implementation of a simple worker:

````c++
void
eventEmittingWorker(Nan::FunctionCallbackInfo<Value> const& args) {
  using namespace std::string_literals;
  unsigned delay      = args[0]->Uint32Value();
  unsigned iterations = args[1]->Uint32Value();

  ncd::AsyncEventEmitter emitter(args[2].As<v8::Object>());  // ①

  ncd::defaultWorkQueue().dispatch([=](){                    // ②
    for (unsigned i = 0; i < iterations; ++i) {
      emitter.emit("progress"s, i);                          // ③
      usleep(delay);
    }
  }, std::bind(emitter.emit, "done"s));                      // ④
}
````

After grabbing some arguments the code creates an `AsyncEventEmitter` in (1). It wraps a javascript `EventEmitter` for use on a different thread. In (2) a lambda expression is dipatched to the threadpool. The expression captures copies of `delay`, `iterations` and the `emitter`. This is an ncd pattern: An `AsyncSomething` is first allocated on the main thread and then copied around to different threads. The call to `dispatch(...)` returns immediately and the lambda is launched on the thredpool. In (3) the async event emitter is invoked, sending progress events back to javascript. After the execution finishes the callback passed in (4) is invoked on the main thread. Here we just emit a done event.

The javascript code looks like this:

````javascript
const workers = require('workers')
  , EventEmitter = require('events')
  , ee = new EventEmitter()
    . on('progress', (p) => { console.log('progress', p) })
    . on('done',     ( ) => { console.log('done') })

workers.eventEmittingWorker(10000, 10, ee)
````

## Concepts

### Callbacks

Like node, ncd uses user supplied callbacks. Alot. The work to be run on the thread and the done handler are callbacks as are the asynchronous requests to the main thread. Basically, everything is a callback. Two basic forms of callbacks are supported: Free functions and callable objects:

````c++
void done() {}
void free_function() {}

struct CallableObject {
  void operator()() {}
};

void
doWork(Nan::FunctionCallbackInfo<Value> const& args) {
  ncd::defaultWorkQueue().dispatch(free_function, done);
  ncd::defaultWorkQueue().dispatch(CallableObject(), done);
}
````

The key difference is that callable objects have state. Note that we actually construct an object in the second call to `dispatch(...)`. We can use it to pass initial parameters to the work we run on a different thread:

````c++
struct WorkWithParam {
  WorkWithParam(int theParam) : mParam(theParam) {}
  void operator()() { std::cerr << "parameter: " << mParam << std::endl; }
  int mParam;
};

void
doWork(Nan::FunctionCallbackInfo<Value> const& args) {
  ncd::defaultWorkQueue().dispatch(WorkWithParam(5), done);
}

````

Unlike more traditional C++ callback APIs, ncd callbacks don't necessarily have a fixed signature. Many of them are very flexible and the user can choose from a number of options. This is documented in the reference. 

#### Lambda Expressions

Since C++11 there is a new way to write C++ callbacks: lambda expressions. Like javasscript functions lambda expressions can be defined inline within another function. Also like with javascript, they can capture values from their parent scopes. Assume the following ES6 arrow function:

````javascript
var f = (status) => { return status >= 0 }
f(5)
````

The corresponding C++ lambda expression looks like this:

````c++
auto f = [](int status) { return status >= 0; };
f(5);
````

We traded an arrow for a pair of brackets, but the similarities are obvious, right? The currently empty brackets are used to control what variables are captured and how. The ncd examples most commonly use a capture list that consists of a single equal sign:

````c++
int minimum = 0;
auto f = [=](int status) { return status >= minimum; };
````

This instructs the compiler to capture all variables we actually use. In javascript that is the default. It will capture them *by copy* and not *by reference*. With ncd the containing function often returns immediately, destroying all local variables. So, copying is the way to go.

Since this is C++ [the actual details of lambda expressions](http://en.cppreference.com/w/cpp/language/lambda) are quite baroque. Please refer to the wider web for additional tutorials.

### Queues

At the core of ncd are two types of code queues. The user dispatches code to a queue and the code is executed on the other side of a thread boundary.

`WorkQueue`s run code on the threadpool. Although there currently is only one default work queue, this will become a fully user construtable type. Each work queue has a maximum number of threads it will use in parallel. A queue with a maximum thread count of one will execute all items sequentially. The default queue is very parallel. It uses `UV_THREADPOOL_SIZE - 4` threads, with a minimum of one.

Code executing on the threadpool has access to an instance of `MainQueue`. Code dispatched on this type of queue is executed on the main thread and can use the javascript engine.

These pairs of queues provide a generic, bidirectional inter-thread facility not unsimilar to apple's grand central dispatch. 

## Handles and Functions

ncd introduces `AsyncHandle<>`s. These handles are used to keep javascript objects alive while work is executing on a thread. This solves many common housekeeping tasks in an unobtrusive way.

`AsyncFunctions` are thread-safe wrappers of javascript functions. They have the syntax of normal function calls but behind the scenes they emit an event on the main thread and invoke the function there.

<p align="center">Docs and tests are AWOL – Please refer to the examples.</p>

