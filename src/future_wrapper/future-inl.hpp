/* Proj: tiny-future
 * File: future-inl.hpp
 * Created Date: 2023/4/20
 * Author: yangyangyang
 * Description:
 * -----
 * Last Modified: 2023/4/20 14:56:17
 * -----
 * Copyright (c) 2023  . All rights reserved.
 */
#ifndef TINY_FUTURE_FUTURE_INL_HPP
#define TINY_FUTURE_FUTURE_INL_HPP

template <typename T>
void Future<T>::setSharedState(const std::shared_ptr<SharedState<T>>& sharedState) {
    assert(sharedState != nullptr);
    sharedState_ = sharedState;
}

template <typename T>
SharedState<T>& Future<T>::getSharedState() noexcept {
    assert(sharedState_ != nullptr);
    return *sharedState_;
}

template <typename T>
void Future<T>::via(Executor* executor) {
    assert(executor != nullptr);
    getSharedState().setExecutor(executor);
}

template <typename T>
template <typename Fn>
void Future<T>::thenValue(Fn&& func) noexcept {
    auto funcWrapper = [f = static_cast<Fn&&>(func)](T&& value) mutable {
        // TODO: invoke-wrapper.
        f(std::move(value));
    };
    Callback callback = [funcWrapper = std::move(funcWrapper)](SharedStateBase& base) mutable {
        auto& sharedState = static_cast<SharedState<T>&>(base);
        funcWrapper(std::move(sharedState.getValue()));
    };
    getSharedState().setCallback(std::move(callback));
}

template <typename T>
void Future<T>::get() && {
    auto&& sharedState = getSharedState();
    assert(sharedState.hasValue());
    sharedState.call();
}

#endif // TINY_FUTURE_FUTURE_INL_HPP
