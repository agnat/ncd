#ifndef NCD_HPP_
# define NCD_HPP_

//=============================================================================
// Not Central Dispatch
//=============================================================================

# include <algorithm>
# include <deque>

# include <iostream>  // XXX

# include <ncd/function.hpp>
# include <ncd/uv.hpp>
# include <ncd/main_queue.hpp>
# include <ncd/work_queue.hpp>

# include <ncd/async_handle.hpp>
# include <ncd/async_function.hpp>
# include <ncd/async_event_emitter.hpp>
# include <ncd/default_work_queue.hpp>

namespace ncd {

//=== Implementations =========================================================

namespace detail {

// This requires definitions of WorkRequestBase, MainQueue, and WorkQueue
inline
void
WorkRequestBase::handleDone(int status) {
  NCD_DBWRK("WorkRequestBase::handleDone() status: " << status);
  mMainQueue->flush();
  onDone();
  mOwner.doneWith(this);
}

# undef NCD_DBWRK

}  // end of namespace detail

}  // end of namespace ncd
#endif  // NCD_HPP_
