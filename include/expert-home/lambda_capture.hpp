// Copyright Felipe Magno de Almeida 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_LAMBDA_CAPTURE_HPP
#define EXPERT_LAMBDA_CAPTURE_HPP

namespace eh {

template <class F>
struct lambda_capture_def
{
  F f; // the lambda will be stored here

  // a forwarding operator():
  template <class... Args>
  decltype(auto) operator()(Args&&... args) const
  {
    // we pass ourselves to f, then the arguments.
    // [edit: Barry] pass in std::ref(*this) instead of *this
    return f(std::ref(*this), std::forward<Args>(args)...);
  }
};

template <class F>
lambda_capture_def<std::decay_t<F>> lambda_capture(F&& f)
{
  return {std::forward<F>(f)};
}

}

#endif
