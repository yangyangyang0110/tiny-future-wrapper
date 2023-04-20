/* Proj: InferenceApi
 * File: random.hpp
 * Created Date: 2022/12/21
 * Author: yangyangyang
 * Description:
 * -----
 * Last Modified: 2022/12/21 16:07:58
 * -----
 * Copyright (c) 2022  . All rights reserved.
 */
#ifndef INFERENCEAPI_RANDOM_HPP
#define INFERENCEAPI_RANDOM_HPP
#include <random>

namespace rng {

inline float randomFloat(float min = 0.0f, float max = 10.0f) noexcept {
    static std::random_device rd;
    static std::mt19937 mt(rd());
    static std::uniform_real_distribution<float> dist(min, max);
    return dist(mt);
}

} // namespace rng

#endif // INFERENCEAPI_RANDOM_HPP
