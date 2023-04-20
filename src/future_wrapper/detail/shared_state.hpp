/* Proj: tiny-future
 * File: shared_state.hpp
 * Created Date: 2023/4/20
 * Author: yangyangyang
 * Description:
 * -----
 * Last Modified: 2023/4/20 14:42:52
 * -----
 * Copyright (c) 2023  . All rights reserved.
 */
#ifndef TINY_FUTURE_SHARED_STATE_HPP
#define TINY_FUTURE_SHARED_STATE_HPP

#include "future_wrapper/define.hpp"
#include "future_wrapper/executor.hpp"

template <typename T>
class SharedState : public SharedStateBase, public MoveOnlyAble, std::enable_shared_from_this<SharedState<T>> {
public:
    using Self = SharedState<T>;
    using SharedPtr = std::shared_ptr<Self>;

    // SharedPtr getPtr() noexcept { return shared_from_this(); }

    void setCallback(Callback&& callback) { callback_ = std::move(callback); }

    template <typename U>
    void setValue(U&& value) {
        value_ = std::forward<U>(value);
        hasValue_ = true;
    }

    bool hasValue() const noexcept { return hasValue_; }

    void setExecutor(Executor* executor) { pExecutor_ = executor; }

    T& getValue() { return value_; }

    void call() {
        if (pExecutor_) {
            // 如何通过后台调用呢？
            pExecutor_->submit([this] { callback_(*this); });
        }
        else {
            callback_(*this);
            // callback_(std::move(value_));
        }
    }

public:
    SharedPtr static Create() noexcept { return std::make_shared<Self>(); }

private:
    // union {
    //     Callback callback_; // 配合shared_ptr构造会失败.
    // };
    Callback callback_;
    Executor* pExecutor_{nullptr};

    bool hasValue_{false};
    T value_;
};

#include <memory>

#endif // TINY_FUTURE_SHARED_STATE_HPP
