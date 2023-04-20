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

/**
 * @brief 模拟folly的Future,
 *  1. SharedState: Promise和Future的共享状态, 用于存储Future设置的回调以及Promise传递的参数.
 *  2. Executor: Future的Runtime或者执行器, 用于将回调函数的执行异步化, 考虑线程池实现
 *  3. Future: 类似stl的future, 增加了thenValue方法, 设置回调函数，由于后面执行
 *  4. Promise: 类似stl的promise,
 * @todo
 *  1. 支持任意类型
 *  2. [支持任意回调类型functor?????()]
 *  3. 何时释放SharedState的堆?
 *  4. Future和Promise的RAII支持
 */

using String = std::string;
using Value = String;
using Callback = std::function<void(Value&& value)>;

struct MoveOnlyAble {
    explicit MoveOnlyAble() noexcept = default;
    MoveOnlyAble(const MoveOnlyAble& other) = delete;
    MoveOnlyAble& operator=(const MoveOnlyAble& other) = delete;
    MoveOnlyAble(MoveOnlyAble&&) noexcept = default;
    MoveOnlyAble& operator=(MoveOnlyAble&&) noexcept = default;
};

// 通过Executor(Future-Runtime)将任务(回调函数)异步运行
class Executor : public MoveOnlyAble {
public:
    virtual ~Executor() noexcept = default;

public:
    virtual void submit(Callback&& callback, Value&& value) = 0;
};

class ThreadExecutor : public Executor {
    using Self = ThreadExecutor;
    using Mutex = std::mutex;
    using task_type = std::function<void()>;

    std::vector<std::thread> threads_;
    std::queue<task_type> task_queue_;
    mutable Mutex mutex_;
    mutable std::condition_variable cv_;
    std::atomic<bool> should_terminate_;
    std::atomic<int32_t> action_thread_;

public:
    void submit(Callback&& callback, Value&& value) final {
        std::lock_guard<Mutex> lock(mutex_);
        task_queue_.emplace([taskPtr = std::make_shared<Callback>(std::move(callback)),
                             value = std::move(value)]() mutable { (*taskPtr)(std::move(value)); });
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
            task_type task;
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

// 共享状态用于存储Future设置的回调函数(及其参数?)以及Promise设置的值
class SharedState /*Core */ : public MoveOnlyAble, std::enable_shared_from_this<SharedState> {
public:
    using Self = SharedState;
    using SharedPtr = std::shared_ptr<Self>;

    SharedPtr getPtr() noexcept { return shared_from_this(); }

    void setCallback(Callback&& callback) { callback_ = std::move(callback); }

    void setValue(Value&& value) {
        value_ = std::move(value);
        hasValue_ = true;
    }

    bool hasValue() const noexcept { return hasValue_; }

    void setExecutor(Executor* executor) { pExecutor_ = executor; }

    Value& getValue() { return value_; }

    void call() {
        if (pExecutor_ != nullptr) {
            pExecutor_->submit(std::move(callback_), std::move(value_));
        }
        else {
            callback_(std::move(value_));
        }
    }

public:
    SharedPtr static Create() noexcept { return std::make_shared<Self>(); }

private:
    Callback callback_;
    Executor* pExecutor_{};

    bool hasValue_{false};
    Value value_;
};

// 模拟
class Future : public MoveOnlyAble {
public:
    Future() noexcept = default;

    void setSharedState(const std::shared_ptr<SharedState>& sharedState) {
        assert(sharedState != nullptr);
        sharedState_ = sharedState;
    }

    SharedState& getSharedState() noexcept {
        assert(sharedState_ != nullptr);
        return *sharedState_;
    }

public:
    void via(Executor* executor) {
        assert(executor != nullptr);
        getSharedState().setExecutor(executor);
    }

    // 透传回调函数等.
    void thenValue(Callback&& callback) noexcept { getSharedState().setCallback(std::move(callback)); }

    // 进行回调的调用
    void get() && {
        auto&& sharedState = getSharedState();
        assert(sharedState.hasValue());
        sharedState.call();
    }

private:
    std::shared_ptr<SharedState> sharedState_;
};

// Promise, 通过setValue设置值
class Promise : public MoveOnlyAble {
public:
    Promise() noexcept { sharedState_ = SharedState::Create(); }

    Future getFuture() {
        Future newFuture{};
        newFuture.setSharedState(sharedState_);
        return newFuture;
    }

    SharedState& getSharedState() noexcept {
        assert(sharedState_ != nullptr);
        return *sharedState_;
    }

public:
    void setValue(Value&& value) { getSharedState().setValue(std::move(value)); };

private:
    std::shared_ptr<SharedState> sharedState_;
};

void printHello(Value&& value) {
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    std::cout << "[" << std::this_thread::get_id() << "] "
              << "Hello: " << value << std::endl;
}

int main() {

    std::unique_ptr<Executor> pExecutor = std::make_unique<ThreadExecutor>(1);
    auto* executor = pExecutor.get();

    Promise p1;
    auto f1 = p1.getFuture();

    Promise p2;
    auto f2 = p2.getFuture();

    f1.via(executor);
    f1.thenValue(printHello);
    p1.setValue("Kitty1");
    std::move(f1).get(); // async.

    f2.via(executor);
    f2.thenValue(printHello);
    p2.setValue("Kitty2");
    std::move(f2).get(); // async.

    std::cout << "[" << std::this_thread::get_id() << "] "
              << "Main Thread say: Bye~" << std::endl;

    return 0;
}
