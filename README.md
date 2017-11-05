# ncd – not central dispatch

ncd is a library to support asynchronous C++ node addons. It provides a number of primitives to work with the UV threadpool. The library uses semantics familiar to every node developer. It allows for a very casual coding style while still facilitating the development of reusable components.

The core principle of ncd is: Pass code, not data. So, instead of using rigid threading and pass around the data the user dispatches code to run on the pool or the main thread respectivley. The code is provided as plain functions, callable objects or lambda expressions.

## Queues

At the heart of ncd are two types of code queues. The user dispatches code to a queue and the code is executed on the other side of a thread boundary. `WorkQueue`s run their code on the threadpool. The executing code in turn has access to an instance of `MainQueue`. Code dispatched on this type of queue executes on the main thread and hence can use the javascript engine.

These pairs of queues provide generic, bidirectional inter-thread communication.

## Handles and Functions

ncd introduces `AsyncHandle<>`s. These handles are used to keep javascript objects alive while work is executing on a thread. This solves many common housekeeping tasks in an unobtrusive way.

