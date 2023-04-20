/* Proj: tiny-future
 * File: promise-inl.hpp
 * Created Date: 2023/4/20
 * Author: yangyangyang
 * Description:
 * -----
 * Last Modified: 2023/4/20 15:09:25
 * -----
 * Copyright (c) 2023  . All rights reserved.
 */
#ifndef TINY_FUTURE_PROMISE_INL_HPP
#define TINY_FUTURE_PROMISE_INL_HPP

template <typename T>
Promise<T>::Promise() noexcept {
    sharedState_ = SharedState<T>::Create();
}

template <typename T>
Future<T> Promise<T>::getFuture() {
    Future<T> newFuture{};
    newFuture.setSharedState(sharedState_);
    return newFuture;
}

template <typename T>
SharedState<T>& Promise<T>::getSharedState() noexcept {
    assert(sharedState_ != nullptr);
    return *sharedState_;
}

template <typename T>
template <typename U>
void Promise<T>::setValue(U&& value) {
    getSharedState().setValue(std::forward<U>(value));
}

#endif // TINY_FUTURE_PROMISE_INL_HPP
