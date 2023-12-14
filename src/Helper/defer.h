#pragma once
#include <type_traits>
#include <utility>

template <typename _Fx>
struct __defer_t {
  _Fx __fx;

  __defer_t(_Fx&& __arg_fx)
  noexcept(::std::is_nothrow_move_constructible_v<_Fx>)
  : __fx(::std::move(__arg_fx)) {}

  ~__defer_t()
  noexcept(::std::is_nothrow_invocable_v<_Fx>) {
    __fx();
  }
};

template <typename _Fx>
__defer_t(_Fx __fx) -> __defer_t<::std::decay_t<_Fx>>;

#define __DEFER_TOK_CONCAT(X, Y) X ## Y
#define __DEFER_TOK_PASTE(X, Y) __DEFER_TOK_CONCAT(X, Y)
#define defer __defer_t \
  __DEFER_TOK_PASTE(__scoped_defer_obj, __COUNTER__) =  \
  [&]()