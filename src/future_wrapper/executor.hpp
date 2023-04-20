/* Proj: tiny-future
 * File: executor.hpp
 * Created Date: 2023/4/20
 * Author: yangyangyang
 * Description:
 * -----
 * Last Modified: 2023/4/20 14:43:58
 * -----
 * Copyright (c) 2023  . All rights reserved.
 */
#ifndef TINY_FUTURE_EXECUTOR_HPP
#define TINY_FUTURE_EXECUTOR_HPP

#include "future_wrapper/define.hpp"
#include <atomic>
#include <boost/type_traits.hpp>

#include <cassert>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

using Func = std::function<void()>;

// 通过Executor(Future-Runtime)将任务(回调函数)异步运行
class Executor : public MoveOnlyAble {
public:
    virtual ~Executor() noexcept = default;

public:
    virtual void submit(Func&& func) = 0;
};

class ThreadExecutor : public Executor {

    using Self = ThreadExecutor;
    using Mutex = std::mutex;

    using WLock = std::unique_lock<Mutex>;
    using WGLock = std::lock_guard<Mutex>;

    std::vector<std::thread> threads_;
    std::queue<Func> task_queue_;
    mutable Mutex mutex_;
    mutable std::condition_variable cv_;
    std::atomic<bool> should_terminate_;
    std::atomic<int32_t> action_thread_;

public:
    // void submit(Callback&& callback, Value&& value) {
    //     std::lock_guard<Mutex> lock(mutex_);
    //     task_queue_.emplace([taskPtr = std::make_shared<Callback>(std::move(callback)),
    //                          value = std::move(value)]() mutable { (*taskPtr)(std::move(value)); });
    //     cv_.notify_one();
    // }

    void submit(Func&& func) final {
        WGLock lock(mutex_);
        task_queue_.emplace(std::forward<Func>(func));
        cv_.notify_one();
    }

    void WaitAndStop() noexcept {
        should_terminate_.store(true);
        cv_.notify_all();
        for (auto& t : threads_) {
            t.join();
        }
        threads_.clear();
    }

public:
    ~ThreadExecutor() noexcept override { WaitAndStop(); }

    void run(std::string const& thread_name) {
        while (true) {
            Func task;
            {
                std::unique_lock<Mutex> lock(mutex_);
                cv_.wait(
                  lock, [this]() { return should_terminate_.load(std::memory_order_relaxed) || !task_queue_.empty(); });
                if (should_terminate_.load(std::memory_order_relaxed) && task_queue_.empty()) {
                    break;
                }
                task = std::move_if_noexcept(task_queue_.front());
                task_queue_.pop();
            }

            ++action_thread_;
            task();
            --action_thread_;
        }
    }

public:
    explicit ThreadExecutor(unsigned int num_thread /*std::thread::hardware_concurrency()*/)
        : should_terminate_(false)
        , action_thread_(0) {
        threads_.reserve(num_thread);
        for (unsigned int i = 0; i < num_thread; ++i) {
            threads_.emplace_back(&Self::run, this, std::string("worker-").append(std::to_string(i)));
        }
    }
};

#endif // TINY_FUTURE_EXECUTOR_HPP
