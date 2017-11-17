// Copyright 2017 David Siegel. Distributed under the MIT license. See LICENSE.
#ifndef NCD_DEFAULT_WORK_QUEUE_HPP_
# define NCD_DEFAULT_WORK_QUEUE_HPP_

# include <ncd/configuration.hpp>

# include <sstream>
# include <iostream>  // XXX

# include <ncd/work_queue.hpp>

namespace ncd {

//=== Default Queue ===========================================================

WorkQueue&
defaultWorkQueue() {
  // TODO: This should be an actual singleton...
  static std::unique_ptr<WorkQueue> sWorkQueue = nullptr;
  if (sWorkQueue == nullptr) {
    const size_t defaultThreadCount = 4;
    size_t threadCount = defaultThreadCount;
    if (char const* threadEnv = std::getenv("UV_THREADPOOL_SIZE")) {
      std::stringstream stream(threadEnv);
      int value;
      stream >> value;
      if (!stream) {
        std::cerr << "Failed to parse UV_THREADPOOL_SIZE: '"
                  << threadEnv << "'" << std::endl;
      } else {
        threadCount = value;
      }
    }
    threadCount = std::max(1ul, threadCount - defaultThreadCount);
    sWorkQueue = std::make_unique<WorkQueue>(uv_default_loop(), threadCount);
  }
  return *sWorkQueue;
}


}  // end of namespace ncd
#endif  // NCD_DEFAULT_WORK_QUEUE_HPP_
