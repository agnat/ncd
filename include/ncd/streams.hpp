#ifndef NCD_STREAMS_HPP_
# define NCD_STREAMS_HPP_

# include <deque>

# include <nan.h>

# include <ncd/uv.hpp>

namespace ncd {

//=============================================================================
// Streams
//=============================================================================

template <typename T>
class StreamBase : Nan::ObjectWrap {
public:
protected:
  using QueueType = std::deque<T>;
  Mutex mMutex;
  Condition mCondition;
  QueueType mQueue;
private:
  size_t mCapacity;
};

template <typename T, typename Converters>
class WritableStream {
public:
protected:
private:
};

}  // end of namespace ncd
#endif  // NCD_STREAMS_HPP_
