**<p align="center">Early draft</p>**

# ncd – not central dispatch

_ncd_ is a library to support asynchronous C++ node addons. It provides a number of primitives to work with the UV thread pool. The library uses semantics familiar to every node developer. It allows for a very casual coding style while still facilitating the development of reusable components.

The core principle of _ncd_ is: Pass code, not data. The user dispatches code to run on the pool or the main thread respectively. The user can choose from plain functions, callable objects or lambda expressions. Anything [callable](http://en.cppreference.com/w/cpp/concept/Callable) will work.

If you are unfamiliar with lambda expressions read the [20 seconds primer](#c-lambda-expression-primer) now.

## Appetizer

Let's take a look at a first example, the implementation of a simple worker:

````c++
void
eventEmittingWorker(Nan::FunctionCallbackInfo<Value> const& args) {
  unsigned delay      = args[0]->Uint32Value();
  unsigned iterations = args[1]->Uint32Value();

  ncd::AsyncEventEmitter emitter(args[2].As<v8::Object>());  // ①

  ncd::defaultWorkQueue().dispatch([=](){                    // ②
    for (unsigned i = 0; i < iterations; ++i) {
      emitter.emit("progress", i);                           // ③
      usleep(delay);
    }
  }, std::bind(emitter.emit, "done"));                       // ④
}
````

After grabbing some arguments the code creates an `AsyncEventEmitter` in (1). It wraps a javascript `EventEmitter` for use on a different thread. In (2) a lambda expression is dipatched to the thread pool. The expression captures copies of `delay`, `iterations` and the `emitter`. This is an _ncd_ pattern: An `AsyncSomething` is first allocated on the main thread and then copied around to different threads. The call to `dispatch(...)` returns immediately and the lambda is launched on the thread pool. In (3) the async event emitter is invoked, sending progress events back to javascript. After the execution finishes the callback passed in (4) is invoked on the main thread. Here we just emit a done event.

The javascript code looks like this:

````javascript
const workers = require('workers')
  , EventEmitter = require('events')
  , ee = new EventEmitter()
    . on('progress', (p) => { console.log('progress', p) })
    . on('done',     ()  => { console.log('done') })

workers.eventEmittingWorker(10000, 10, ee)
````

## Concepts

### Callbacks

Like node, _ncd_ uses callbacks. Alot. The code to be run on the thread and the done handler are callbacks as are the asynchronous requests to the main thread. Basically, everything is a callback. Two basic forms of callbacks are supported: Free functions and callable objects:

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

Unlike more traditional C++ callback APIs, _ncd_ callbacks don't necessarily have a fixed signature. Some of them are very flexible and the user can choose from a number of options. This is documented in the reference.

Since _ncd_ is intentionally unspecific about the callback types virtually all C++ callables will work as long as they have a matching signature. This includes callables returned by C++ standard utilities like `std::bind(...)`. In the [appetizer](#appetizer) above an event name is tied to the asynchronous event emitter. The result is directly used as a done handler.

#### Lambda Expressions

What about lambda expressions? Isn't that a third kind of callback? Well, yes. You can use lambda expressions as callbacks as shown in the first example. And no, they are not a new kind of callable. What happens is this: The compiler analyzes the lambda and checks if it captures any variables. If it does not capture anything the compiler creates a hidden function. Else it creates a hidden class for a callable object. The object has the necessary members to hold the captured values.

The [basic work example](https://github.com/agnat/ncd/tree/master/examples/01.basic_work) covers all three callback flavours.

### Queues

At the core of _ncd_ are two types of queues. The user dispatches code to a queue and the code is executed on the other side of a thread boundary.

`WorkQueue`s run code on the thread pool. Although there currently is only one default work queue, this will become a fully user constructable type. Each work queue has a maximum number of threads it will use in parallel. A queue with a maximum thread count of one will execute all items sequentially while queues with more threads execute items concurrently. The default queue uses `UV_THREADPOOL_SIZE` - 4 threads, with a minimum of one. That is, it is a sequential queue until you start to give it more threads by setting `UV_THREADPOOL_SIZE`.

Code executing on the thread pool has access to an instance of `MainQueue`. Code dispatched on this type of queue is executed on the main thread and can use the javascript engine.

````c++
std::thread::id threadId() { return std::this_thread::get_id(); }

void
workFunction(Nan::FunctionCallbackInfo<Value> const& args) {
  std::cerr << "main thread " << threadId() << std::endl;
  
  ncd::defaultWorkQueue().dispatch([](){
    std::cerr << "pool thread " << threadId() << std::endl;
  
    ncd::mainQueue().dispatch([](){
      std::cerr << "main thread " << threadId() << std::endl;
    });
  }, done);
}
````

These pairs of queues provide a generic, bidirectional inter-thread facility not unsimilar to apple's grand central dispatch. Both, the apple and the libuv event loop have a notion of a main thread. They both provide a thread pool. However, as the name suggests the similarities only go so far. 

If you are familiar with libuv programming you already guessed it. Behind the scenes the work queue deals with items of type `uv_work_t`. The main queue handles the other direction and holds a `uv_async_t` that is the backchannel for the current work function.

#### Queue Invariants

 - `WorkQueue`s always _launch_ their work in sequential order
 - `WorkQueue`s with a single thread are strictly sequential. The previous item is finished before the next item is launched.
 - `MainQueue`s are always strictly sequential
 - Additionally, all items dispatched on a `MainQueue` are executed before the done handler.

### Handles

_ncd_ introduces `AsyncHandle<>`s. These handles are used to keep javascript objects alive while work is executing on a thread. This solves many common housekeeping tasks in an unobtrusive way:

````c++
void
workFunction(Nan::FunctionCallbackInfo<Value> const& args) {
  ncd::AsyncHandle<v8::Object> pinnedObject(args[0].As<v8::Object>());  // ①
  
  ncd::defaultWorkQueue().dispatch([=](){                               // ②
    for (unsigned i = 0; i < 10; ++i) {
      usleep(10000);
      ncd::mainQueue().dispatch([=](){                                  // ③
        Nan::HandleScope scope;
        v8::Local<v8::Object> object = pinnedObject.jsValue();          // ④
        object->Set(Nan::New("progress").ToLocalChecked(), Nan::New(i));
      });
    }
  }, done);
}

````

The code creates an `AsyncHandle<v8::Object>` in (1) pinning an object for later use. The handle is copied
to the worker thread in (2) and back to the main thread in (3). Finally, back on the main thread the javascript value is extracted in (4). The code above can be found in the [main queue examples](https://github.com/agnat/ncd/tree/master/examples/03.main_queue).

## Utilities

`AsyncFunctions` are thread-safe wrappers of javascript functions. They are invoked like regular functions but behind the scenes they emit an event on the main thread and invoke the javascript function there.

<p align="center">Docs and tests are AWOL – Please refer to the examples.</p>

## Appendices

### C++ Lambda Expression Primer

Since C++11 there is a new way to write C++ callbacks: lambda expressions. Like javasscript functions lambda expressions can be defined inline within another function. Also like with javascript, they can capture values from their parent scopes. Assume the following ES6 arrow function:

````javascript
var f = (status) => { return status >= 0 }
f(5)
````

The equivalent C++ lambda expression looks like this:

````c++
auto f = [](int status) { return status >= 0; };
f(5);
````

We traded an arrow for a pair of brackets, but the similarities are obvious, right? The currently empty brackets are used to tell the compiler what variables are captured and how. It is called the _capture list_. The examples most commonly use a capture list that consists of a single equal sign:

````c++
int minimum = 0;
auto f = [=](int status) { return status >= minimum; };
````

This instructs the compiler to capture all variables we use. In javascript that is the default. It will capture them *by copy* and not *by reference*. The lambda above holds a copy of `minimum`. With _ncd_ the containing function often returns immediately, destroying all local variables. So, copying is the way to go.

Since this is C++ [the actual details of lambda expressions](http://en.cppreference.com/w/cpp/language/lambda) are quite baroque. Please refer to the wider web for additional tutorials.

### Design Notes

_ncd_ hides many of the more daunting details of libuv/C++ multi-threading. It employs C++ best practices to arrive at an API that feels familiar to node/javascript developers. The goal here is to be as javascripty as possible _without_ creating awkward C++ or (unwarrented) overhead. It aims to be a semantic fusion without bending things to far.

  1. By using C++ callables as the core abstraction, we gain flexibility. Mixing and matching free functions, callable objects and lambda expressions gives us a semantic reach similar to javascript functions. We can keep state, use inline syntax and capture variables. _ncd_ also follows nodes signature for callbacks: error first, result second.
  1. _ncd_ tries to avoid burdening the user with housekeeping tasks. It mostly uses automatic RAII-style memory management. In general it avoids user-facing symmetries like new/delete, lock/unlock, &c. Anything the user has to remember is bad, because we tend to forget.
  1. _ncd_ directly implements a number of high level components from the node universe, event emitters and streams and makes them available to multi-threaded addons. The hope is that reusing these familiar async primitives will make it easier to reason about these addons.

#### Code Walkthrough

_ncd_ core consists of three basic primitives: `WorkQueue`, `MainQueue`, and `AsyncHandle`. Everything else is either standalone or build on top of the core. 

##### Main Queue

A natural place to start reading is _work_queue.hpp_. The queue files each contain the actual queue class like `WorkQueue` and the items managed by that queue: `WorkRequestBase` and the derieved template `WorkRequest<...>. This is an interesting (albeit not new) pattern. A template derieves from an abstract base class filling in the pure virtual functions:

````c++
struct Base {
  virtual ~Base() {}
  virtual void doThing() = 0;
};

template <typename Callable>
struct Derieved : Base {
  void doThing() override { /* do things depending on Callable */ }
};
````

Note how this is similar to the current `AsyncWorker`. Except that from the users point of view the derieved type is _automatically generated_ to fit the callable. Think of the pattern as a polymorphic factory. Given this setup, we can stuff pointers to the base class into standard containers and do things with them without knowing about the actual callable.

##### Work Queue

The class `WorkRequestBase` manages the `MainQueue` for a given work function. Looking through _main_queue.hpp_ you'll see it's much less complex and uses a similar pattern like the `WorkQueue`. 

[modeline]: # ( vim: set fenc=utf-8 spell spelllang=en_us: )
