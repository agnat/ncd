// Copyright 2017 David Siegel. Distributed under the MIT license. See LICENSE.
#ifndef NCD_HPP_
# define NCD_HPP_

# include <ncd/function.hpp>
# include <ncd/uv.hpp>

# include <ncd/main_queue.hpp>
# include <ncd/work_queue.hpp>
# include <ncd/dispatch.hpp>

# include <ncd/default_work_queue.hpp>
# include <ncd/queue_getters.hpp>

# include <ncd/async_handle.hpp>
# include <ncd/async_buffer.hpp>
# include <ncd/async_function.hpp>
# include <ncd/async_event_emitter.hpp>

# include <ncd/streams.hpp>

# define NCD_NODE_ADDON(init) NODE_MODULE(NODE_GYP_MODULE_NAME, init)

#endif  // NCD_HPP_
