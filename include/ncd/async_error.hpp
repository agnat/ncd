// Copyright 2017 David Siegel. Distributed under the MIT license. See LICENSE.
#ifndef NCD_ASYNC_ERROR_HPP_
# define NCD_ASYNC_ERROR_HPP_

# include <ncd/configuration.hpp>

# include <string>

namespace ncd {

struct AsyncError {
public:
  AsyncError() : mMessage() {}
  AsyncError(std::string const& message) : mMessage(message) {}

  std::string const&
  message() const { return mMessage; }
private:
  std::string mMessage;
};

}  // end of namespace ncd
#endif  // NCD_ASYNC_ERROR_HPP_
