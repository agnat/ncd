// Copyright 2017 David Siegel. Distributed under the MIT license. See LICENSE.
#ifndef NCD_META_HPP_
# define NCD_META_HPP_

# include <ncd/configuration.hpp>

# include <functional>
# include <tuple>

namespace ncd {

namespace detail {

// https://stackoverflow.com/questions/11893141/inferring-the-call-signature-of-a-lambda-or-arbitrary-callable-for-make-functio


//==============================================================================
// Meta function to strip the class of a given member function type. Invoked on
// a member function pointer type it returns a matching function type preserving
// arguments and return type.
//==============================================================================
template <typename T> struct strip_class;

template <typename C, typename R, typename... A>
struct strip_class<R(C::*)(A...)> { using type = R(A...); };

template <typename C, typename R, typename... A>
struct strip_class<R(C::*)(A...) const> { using type = R(A...); };

template <typename C, typename R, typename... A>
struct strip_class<R(C::*)(A...) volatile> { using type = R(A...); };

template <typename C, typename R, typename... A>
struct strip_class<R(C::*)(A...) const volatile> { using type = R(A...); };


//==============================================================================
// Meta function to get the raw function signature of all the callable things.
//==============================================================================
template <typename T>
struct callable_signature {
  using type = typename strip_class<
    decltype(&std::remove_reference<T>::type::operator())>::type;
};

template <typename C, typename R, typename... Args>
struct callable_signature<R(C::*)(Args...)> {
  using type = R(C *, Args...);
};

template <typename C, typename R, typename... Args>
struct callable_signature<R(C::*)(Args...) const> {
  using type = R(C *, Args...);
};

template <typename C, typename R, typename... Args>
struct callable_signature<R(C::*)(Args...) volatile> {
  using type = R(C *, Args...);
};

template <typename C, typename R, typename... Args>
struct callable_signature<R(C::*)(Args...) const volatile> {
  using type = R(C *, Args...);
};

template <typename R, typename... A>
struct callable_signature<R(A...)> { using type = R(A...); };

template <typename R, typename... A>
struct callable_signature<R(&)(A...)> { using type = R(A...); };

template <typename R, typename... A>
struct callable_signature<R(*)(A...)> { using type = R(A...); };

} // end of namespace detail


// Generic utilities to construct std::function wrappers on callable things

template <typename T> using callable_signature = typename detail::callable_signature<T>::type;

template <typename F> using make_function_type = std::function<callable_signature<F>>;

template<typename F> 
make_function_type<F>
make_function(F &&f) {
  return make_function_type<F>(std::forward<F>(f));
}

namespace detail {

template <int...> struct seq {};

template <int N, int... S> struct gens : gens<N-1, N-1, S...> {};
template <int... S> struct gens<0, S...> { using type = seq<S...>; };

} // end of namespace detail


} // end of namespace ncd
#endif // NCD_META_HPP_
