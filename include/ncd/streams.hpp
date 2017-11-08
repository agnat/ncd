// Copyright 2017 David Siegel. Distributed under the MIT license. See LICENSE.
#ifndef NCD_STREAMS_HPP_
# define NCD_STREAMS_HPP_

# include <deque>
# include <iostream> // XXX

# include <ncd/uv.hpp>
# include <ncd/js_class.hpp>
# include <ncd/async_handle.hpp>
# include <ncd/converters.hpp>

namespace ncd {

//=============================================================================
// Streams
//=============================================================================

template <typename T, typename Derived>
class StreamBase : public JSWrapped<Derived> {
public:

protected:
  using QueueType = std::deque<T>;
  QueueType& queue()     { return mQueue; }
  Mutex&     mutex()     { return mMutex; }
  Condition& condition() { return mCondition; }

private:
  Mutex     mMutex;
  Condition mCondition;
  QueueType mQueue;
  size_t    mCapacity;
};

class AsyncStreamBase {
public:
  AsyncStreamBase(v8::Local<v8::Object> jsStream) : mStream(jsStream) {}
private:
  AsyncHandle<v8::Object> mStream;
};

template <typename T>
class AsyncReadStream : public AsyncStreamBase {
  using AsyncStreamBase::AsyncStreamBase;
};

template <typename T, typename Converters = ConvenientConverters>
class WritableStream : public StreamBase<T, WritableStream<T, Converters>> {
public:
  using Base = StreamBase<T, WritableStream<T, Converters>>;
  using AsyncEndpoint = AsyncReadStream<T>;

public:
  WritableStream(Nan::FunctionCallbackInfo<v8::Value> const&) {
    std::cerr << "WritableStream()" << std::endl;
  }

  static
  void
  makeTemplate(v8::Local<v8::FunctionTemplate> tpl) {
    Base::prototypeMethod("_write", &WritableStream::_write);
  }

protected:

private:
  void
  _write(Nan::FunctionCallbackInfo<v8::Value> const& args) {
    std::cerr << "_write()" << std::endl;
  }
private:
};

}  // end of namespace ncd
#endif  // NCD_STREAMS_HPP_
