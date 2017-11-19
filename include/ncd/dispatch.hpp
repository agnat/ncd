#ifndef NCD_DISPATCH_HPP_
# define NCD_DISPATCH_HPP_

# include <ncd/configuration.hpp>

# include <utility>

# include <ncd/work_queue.hpp>
# include <ncd/main_queue.hpp>

namespace ncd {

template <typename Work, typename Callback>
void
dispatch(WorkQueue & queue, Work && work, Callback && callback) {
  queue.push(std::forward<Work>(work), std::forward<Callback>(callback));
}

template <typename Work>
void
dispatch(WorkQueue & queue, Work && work, v8::Local<v8::Function> callback) {
  queue.push(std::forward<Work>(work), callback);
}

template <typename Callback>
void
dispatch(MainQueue & queue, Callback && callback) {
  queue.push(std::forward<Callback>(callback));
}

}  // end of namespace ncd
#endif  // NCD_DISPATCH_HPP_
