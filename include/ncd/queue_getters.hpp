#ifndef NCD_QUEUE_GETTERS_HPP_
# define NCD_QUEUE_GETTERS_HPP_

# include <ncd/uv.hpp>
# include <ncd/main_queue.hpp>

namespace ncd {

class MainQueue;

namespace detail {

struct MainQueueTLSKey {};
using TLSMainQueue = ThreadLocal<MainQueueTLSKey, MainQueue>;

}  // end of namespace detail

// result is valid in work functions only
inline
MainQueue&
mainQueue() {
  auto queue = detail::TLSMainQueue::get();
  assert(queue != nullptr);
  return *queue;
}

namespace detail {
bool
isMainThread() {
  return detail::TLSMainQueue::get() == nullptr;
}
}  // end of namespace detail

}  // end of namespace ncd
#endif  // NCD_QUEUE_GETTERS_HPP_
