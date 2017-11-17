#ifndef NCD_PLATFORM_HPP_
# define NCD_PLATFORM_HPP_

# include <memory>

# if __cplusplus < 201103L
#  error "ncd requires C++11 or better
# endif

namespace ncd {
# if __cplusplus == 201103L

template <typename T, typename... Args>
std::unique_ptr<T>
make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

# else

template <typename T, typename... Args>
std::unique_ptr<T>
make_unique(Args&&... args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

# endif

}  // end of namespace ncd
#endif  // NCD_PLATFORM_HPP_
