/* Proj: InferenceApi
 * File: literal.hpp
 * Created Date: 2022/12/23
 * Author: yangyangyang
 * Description:
 * -----
 * Last Modified: 2022/12/23 11:23:12
 * -----
 * Copyright (c) 2022  . All rights reserved.
 */
#ifndef INFERENCEAPI_LITERAL_HPP
#define INFERENCEAPI_LITERAL_HPP

#include <string>

static inline std::string operator"" _str(const char* bytes, size_t size) noexcept {
    return {bytes, size};
}

#endif // INFERENCEAPI_LITERAL_HPP
