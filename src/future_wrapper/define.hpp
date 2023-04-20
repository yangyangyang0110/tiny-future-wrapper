/* Proj: tiny-future
 * File: define.hpp
 * Created Date: 2023/4/20
 * Author: yangyangyang
 * Description:
 * -----
 * Last Modified: 2023/4/20 14:44:13
 * -----
 * Copyright (c) 2023  . All rights reserved.
 */
#ifndef TINY_FUTURE_DEFINE_HPP
#define TINY_FUTURE_DEFINE_HPP

#include <functional>
#include <string>

using String = std::string;
using Value = String;

class SharedStateBase {};

// using Callback = std::function<void(SharedStateBase&)>;
using Callback = std::function<void(Value&&)>;

struct MoveOnlyAble {
    explicit MoveOnlyAble() noexcept = default;
    MoveOnlyAble(const MoveOnlyAble& other) = delete;
    MoveOnlyAble& operator=(const MoveOnlyAble& other) = delete;
    MoveOnlyAble(MoveOnlyAble&&) noexcept = default;
    MoveOnlyAble& operator=(MoveOnlyAble&&) noexcept = default;
};

#endif // TINY_FUTURE_DEFINE_HPP
