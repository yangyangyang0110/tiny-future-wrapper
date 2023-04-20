/* Proj: tiny-future
 * File: promise.hpp
 * Created Date: 2023/4/20
 * Author: yangyangyang
 * Description:
 * -----
 * Last Modified: 2023/4/20 14:46:18
 * -----
 * Copyright (c) 2023  . All rights reserved.
 */
#ifndef TINY_FUTURE_PROMISE_HPP
#define TINY_FUTURE_PROMISE_HPP

#include "./define.hpp"

template <typename T>
class Promise : public MoveOnlyAble {
public:
    Promise() noexcept;

    Future<T> getFuture();

    SharedState<T>& getSharedState() noexcept;

public:
    template <typename U>
    void setValue(U&& value);

private:
    std::shared_ptr<SharedState<T>> sharedState_;
};

#include <future_wrapper/promise-inl.hpp>

#endif // TINY_FUTURE_PROMISE_HPP
