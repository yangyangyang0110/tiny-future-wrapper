/* Proj: InferenceApi
 * File: os.hpp
 * Created Date: 2022/12/11
 * Author: yangyangyang
 * Description:
 * -----
 * Last Modified: 2022/12/11 16:56:35
 * -----
 * Copyright (c) 2022  . All rights reserved.
 */

#pragma once

#include "./literal.hpp"
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#ifndef OS_SEP
#ifdef __MSC_VER__
#define OS_SEP '\\'
#else
#define OS_SEP '/'
#endif
#endif

inline std::string addSepIfNot(std::string&& str, char sep = OS_SEP) noexcept {
    if (str.empty())
        return str;

    if (str[str.size() - 1] != sep) {
        str.push_back(sep);
    }

    return str;
}

inline std::string getLastDirName(const std::string& filepath, char sep = OS_SEP) noexcept {
    if (filepath.empty())
        return {};

    size_t pos;
    if ((pos = filepath.find_last_of(sep)) != std::string::npos) {
        return filepath.substr(pos + 1, filepath.size() - pos);
    }
    return filepath;
}

template <size_t N, typename Array = std::array<std::string, N>>
inline std::string join(const Array& arr, char sep = OS_SEP) noexcept {
    if (arr.empty())
        return {};
    std::stringstream ss;
    for (int32_t i = 0; i < arr.size(); ++i) {
        ss << arr[i];
        if (i < arr.size() - 1)
            ss << sep;
    }
    return ss.str();
}

template <typename Element = std::string, typename Vector = std::vector<Element>>
inline std::string join(const Vector& vec, const Element& delim) noexcept {
    std::stringstream ss;
    int32_t idx = 0;
    std::for_each(vec.cbegin(), vec.cend(), [&, idx](const std::string& str) mutable {
        ss << str << ((++idx < vec.size()) ? delim : Element{});
    });
    return ss.str();
}

inline bool isFile(const std::string& filepath) noexcept {
    if (filepath.empty())
        return false;
    // RAII.
    const std::ifstream checkFile(filepath);
    return checkFile.is_open();
}

inline bool isDir(const std::string& dir_path) noexcept {
    if (dir_path.empty())
        return false;

    DIR* dir_handle = opendir(dir_path.c_str());
    const bool isDir = dir_handle != nullptr;
    if (isDir) {
        closedir(dir_handle);
    }
    return isDir;
}

inline bool removeDirectory(const std::string& dir_path) noexcept {
    if (!isDir(dir_path))
        return false;

    DIR* dir_handle;
    struct dirent* entry;
    if ((dir_handle = opendir(dir_path.c_str())) != nullptr) {
        while ((entry = readdir(dir_handle)) != nullptr) {
            // ignore . and ..
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            // get sub abspath.
            const auto abs_path = join<2>({dir_path, entry->d_name});

            // sub fd is dir or file?
            if (isDir(abs_path)) {
                removeDirectory(abs_path);
            }
            else if (isFile(abs_path)) {
                remove(abs_path.c_str());
            }
        }
        closedir(dir_handle);
        remove(dir_path.c_str());
    }

    return true;
}

void getSubDirs(std::vector<std::string>& out, const std::string& in, bool is_recursive) noexcept {
    // open dir.
    DIR* dir_handle;
    struct dirent* entry;

    if ((dir_handle = opendir(in.c_str())) != nullptr) {
        // loop to get sub dir.
        while ((entry = readdir(dir_handle)) != nullptr) {
            // ignore . and ..
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                std::string sub_path = join<2>({in, entry->d_name});
                if (isDir(sub_path)) {
                    if (is_recursive) {
                        getSubDirs(out, sub_path, is_recursive);
                    }
                    out.push_back(std::move(sub_path));
                }
            }
        }
        closedir(dir_handle);
    }
}

std::vector<std::string> listSubDir(const std::string& dir, bool is_sort_result = true) noexcept {
    if (!isDir(dir)) {
        return {};
    }

    std::vector<std::string> abs_paths;
    getSubDirs(abs_paths, dir, false);

    if (is_sort_result) {
        std::sort(abs_paths.begin(), abs_paths.end());
    }

    return abs_paths;
}
