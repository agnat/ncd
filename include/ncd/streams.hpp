// Copyright 2017 David Siegel. Distributed under the MIT license. See LICENSE.
#ifndef NCD_STREAMS_HPP_
# define NCD_STREAMS_HPP_

# include <ncd/configuration.hpp>

# include <deque>
# include <iostream> // XX

# include <ncd/uv.hpp>
# include <ncd/js_class.hpp>
# include <ncd/async_handle.hpp>
# include <ncd/converters.hpp>
# include <ncd/async_value.hpp>
# include <ncd/function.hpp>
# include <ncd/queue_getters.hpp>

namespace ncd {

//=============================================================================
// Streams
//=============================================================================

template <typename ValueType, typename Derived>
class StreamBase : public JSWrapped<Derived> {
public:
  explicit
  StreamBase(Nan::FunctionCallbackInfo<v8::Value> const& args)
    : mCapacity(16)
  {
    if (args.Length() > 0 && args[0]->IsObject()) {
      v8::Local<v8::Object> options = args[0].As<v8::Object>();
      v8::Local<v8::Value> objectMode_ = options->Get(v8str("objectMode"));
      v8::Local<v8::Value> highWaterMark = options->Get(v8str("highWaterMark"));
      if (!objectMode_.IsEmpty() && objectMode_->IsTrue() && 
          !highWaterMark.IsEmpty() && highWaterMark->IsUint32())
      {
        mCapacity = highWaterMark->Uint32Value();
      }
    }
  }

protected:
  using QueueType = std::deque<ValueType>;
  QueueType& queue()     { return mQueue; }
  Mutex&     mutex()     { return mMutex; }
  Condition& condition() { return mCondition; }

  // callers are required to hold the mutex
  bool isEmpty() const { return mQueue.empty(); }
  bool isFull()  const { return mQueue.size() >= mCapacity; }

private:
  Mutex     mMutex;
  Condition mCondition;
  QueueType mQueue;
  size_t    mCapacity;
};

template <typename Stream>
class AsyncStream {
public:
  explicit
  AsyncStream(v8::Local<v8::Object> jsStream) 
    : mJSStream(jsStream), mStream(*Stream::unwrap(jsStream))
  {}

  Stream&
  stream() const { return mStream; }

private:
  AsyncHandle<v8::Object> mJSStream;
  Stream & mStream;
};

template <typename Stream>
class AsyncReader {
public:
  AsyncReader(AsyncStream<Stream> const& stream)
    : mStream(stream.stream())
  {}
  using ValueType = typename Stream::ValueType;

  ValueType
  readSync() const {
    return mStream.readSync();
  }
private:
  Stream & mStream;
};

template <typename T, typename Converters = ConvenientConverters>
class WritableStream 
  : public StreamBase<AsyncValue<T, Converters>, WritableStream<T, Converters>> 
{
public:  // types
  using ValueType = AsyncValue<T, Converters>;
  using Self = WritableStream<T, Converters>;
  using Base = StreamBase<ValueType, Self>;
  using AsyncStream = AsyncStream<Self>;
  using AsyncEndpoint = AsyncReader<Self>;

public:
  explicit
  WritableStream(Nan::FunctionCallbackInfo<v8::Value> const& args)
    : Base(args)
  {
    std::cerr << "WritableStream()" << std::endl;
  }

  static
  void
  makeClass(v8::Local<v8::FunctionTemplate> tpl) {
    Base::prototypeMethod("_write", &WritableStream::_write);
    Base::prototypeMethod("_final", &WritableStream::_final);
  }

  ValueType
  readSync() {
    bool queueWasFull;
    ValueType result = readSync(queueWasFull);
    if (queueWasFull || ! result) {
      mainQueue().dispatch([this]() { mCallback(); });
    }
    return result;
  }

private:
  void
  _write(Nan::FunctionCallbackInfo<v8::Value> const& args) {
    std::cerr << "_write()" << std::endl;
    Nan::HandleScope scope;
    ValueType chunk(args[0]);
    v8::Local<v8::String> encoding = args[1].As<v8::String>();
    Function<void()> writeCallback(args[2].As<v8::Function>());
    bool isFull = write(chunk);
    if (isFull) {
      mCallback = writeCallback;
    } else {
      writeCallback();
    }
  }
  void
  _final(Nan::FunctionCallbackInfo<v8::Value> const& args) {
    std::cerr << "_final()" << std::endl;
    Nan::HandleScope scope;
    mCallback = Function<void()>(args[0].As<v8::Function>());
    write(ValueType::sentinel());
  }

private:
  bool
  write(ValueType const& value) {
    ScopedLock scopedLock(this->mutex());
    this->queue().push_back(value);
    bool isFull = this->isFull();
    this->condition().broadcast();
    return isFull;
  }

  ValueType
  readSync(bool & wasFull) {
    ScopedLock scopedLock(this->mutex());
    while (this->isEmpty()) {
      this->condition().wait(this->mutex());
    }
    wasFull = this->isFull();
    ValueType result = this->queue().front();
    this->queue().pop_front();
    return result;
  }

private:
  Function<void()> mCallback;
};

}  // end of namespace ncd
#endif  // NCD_STREAMS_HPP_
