/* Proj: InferenceApi
 * File: os_test.cpp
 * Created Date: 2022/12/23
 * Author: yangyangyang
 * Description:
 * -----
 * Last Modified: 2022/12/23 11:20:05
 * -----
 * Copyright (c) 2022  . All rights reserved.
 */

#include "./os.hpp"
#include <cstring>
#include <gtest/gtest.h>

TEST(LIST_DIR, GET_SUB_DIR) {
    const auto inputDir = "/home/ubuntu/workspace/cpp/InferenceApi/logs-"_str;
    auto subDirs = listSubDir(inputDir);
    for (auto&& subDir : subDirs) {
        std::cout << subDir << std::endl;
    }
}

TEST(CSTRING, STRCMP) {
    char dir_name[] = "..";
    EXPECT_TRUE(strcmp(dir_name, "..") == 0);
    EXPECT_TRUE(strcmp(dir_name, ".") != 0);
}

TEST(JOIN, ACTUAL) {
    auto res = join({"hello"_str, "world"_str}, "--"_str);
    EXPECT_EQ(res, "hello--world"_str);
}

TEST(getLastDirName, ACTUAL) {
    auto inp = "logs"_str;
    EXPECT_EQ(getLastDirName(inp), "logs");

    inp = "logs/2022-12-23"_str;
    EXPECT_EQ(getLastDirName(inp), "2022-12-23");

    inp = "logs/2022-12-23/"_str;
    EXPECT_EQ(getLastDirName(inp), "");

    inp = "logs/2022-12-23/xx"_str;
    EXPECT_EQ(getLastDirName(inp), "xx");

    inp = "logs/2022-12-23/xx.@#$@%$$#%$$%^%$^%^"_str;
    EXPECT_EQ(getLastDirName(inp), "xx.@#$@%$$#%$$%^%$^%^");
}

TEST(REMOVE_DIR, ACTUAL) {
    const auto inputDir = "/home/ubuntu/workspace/cpp/InferenceApi/logs-"_str;
    EXPECT_TRUE(removeDirectory(inputDir));
}
