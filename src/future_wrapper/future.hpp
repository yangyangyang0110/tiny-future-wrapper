/* Proj: tiny-future
 * File: future.hpp
 * Created Date: 2023/4/20
 * Author: yangyangyang
 * Description:
 * -----
 * Last Modified: 2023/4/20 14:41:06
 * -----
 * Copyright (c) 2023  . All rights reserved.
 */
#ifndef TINY_FUTURE_FUTURE_HPP
#define TINY_FUTURE_FUTURE_HPP

#include "future_wrapper/define.hpp"
#include "future_wrapper/detail/shared_state.hpp"
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

template <typename T>
class Future : public MoveOnlyAble {
public:
    Future() noexcept = default;

    void setSharedState(const std::shared_ptr<SharedState<T>>& sharedState);

    SharedState<T>& getSharedState() noexcept;

public:
    void via(Executor* executor);

    void thenValue(Callback&& callback) noexcept;

    void get() &&;

private:
    std::shared_ptr<SharedState<T>> sharedState_;
};

#include "future_wrapper/future-inl.hpp"

#endif // TINY_FUTURE_FUTURE_HPP
