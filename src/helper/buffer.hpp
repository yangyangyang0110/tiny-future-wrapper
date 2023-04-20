#pragma once

#include <boost/circular_buffer.hpp>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

class MoveOnlyAble {
public:
    explicit MoveOnlyAble() noexcept = default;
    MoveOnlyAble(MoveOnlyAble&&) noexcept = default;
    MoveOnlyAble& operator=(MoveOnlyAble&&) noexcept = default;
};

template <typename T>
class RingBuffer : public MoveOnlyAble {
public:
    explicit RingBuffer(size_t size)
        : m_data(nullptr)
        , m_capacity(0)
        , m_head(0)
        , m_tail(0)
        , m_size(0) {

        if (size <= 0) {
            throw std::invalid_argument("size must be non-negative");
        }
        m_data = new T[size];
        m_capacity = size;
    }

    ~RingBuffer() noexcept {
        if (m_data != nullptr) {
            delete[] m_data;
            m_data = nullptr;
        }
        m_head = m_tail = m_size = m_capacity = 0;
    }

    template <class U>
    bool try_push(U&& val) noexcept {
        if (full()) {
            return false;
        }
        m_data[m_tail] = std::forward<U>(val);
        increment_pos(m_tail);
        ++m_size;
        return true;
    }

    bool try_pop(T& val) noexcept {
        if (empty()) {
            return false;
        }
        val = std::move_if_noexcept(m_data[m_head]);
        increment_pos(m_head);
        --m_size;
        return true;
    }

    inline bool full() const noexcept { return m_size == m_capacity; };

    inline bool empty() const noexcept { return m_size == 0; };

    inline size_t size() const noexcept { return m_size; }

    inline size_t captity() const noexcept { return m_capacity; }

private:
    inline void increment_pos(size_t& pos) noexcept { pos = (pos + 1) % m_capacity; }

private:
    T* m_data;
    size_t m_head, m_tail, m_size, m_capacity;
};

template <typename T>
class ThreadSafeQueue {
public:
    using Value = T;
    using Self = ThreadSafeQueue;

    explicit ThreadSafeQueue(size_t capacity)
        : buffer_{capacity} {}

    template <typename... Args, typename = typename std::enable_if<std::is_constructible<Args..., T>::value>::type>
    void wait_and_emplace(Args&&... args) {
        std::lock_guard<std::mutex> lk(mutex_);
        not_full_.wait(lk, [this]() { return !buffer_.full(); });
        buffer_.emplace(std::forward<Args>(args)...);
        not_empty_.notify_one();
    }

    template <typename... Args, typename = typename std::enable_if<std::is_constructible<Args..., T>::value>::type>
    bool try_emplace(Args&&... args) {
        std::lock_guard<std::mutex> lk(mutex_);
        if (buffer_.full()) {
            return false;
        }
        buffer_.emplace(std::forward<Args>(args)...);
        not_empty_.notify_one();
        return true;
    }

    template <typename U>
    void wait_and_push(U&& val) noexcept {
        std::unique_lock<std::mutex> lk{mutex_};
        not_full_.wait(lk, [this]() { return !buffer_.full(); });
        buffer_.push_back(std::forward<U>(val));
        not_empty_.notify_one();
    }

    template <typename U>
    bool try_push(U&& val) noexcept {
        std::unique_lock<std::mutex> lk{mutex_};
        if (buffer_.full()) {
            return false;
        }
        buffer_.push_back(std::forward<U>(val));
        not_empty_.notify_one();
        return true;
    }

    bool try_pop(Value& res) noexcept {
        std::lock_guard<std::mutex> lk(mutex_);
        if (buffer_.empty())
            return false;
        res = std::move_if_noexcept(buffer_.back());
        buffer_.pop_back();
        not_full_.notify_one();
        return true;
    }

    // template <class Rep, class Period>
    // std::vector<Value> wait_and_pop(const std::chrono::duration<Rep, Period>& timeout) noexcept {
    //     std::unique_lock<std::mutex> lk(mutex_);
    //     not_empty_.wait_for(lk, timeout, [this] { return !buffer_.empty(); });
    //     Value res = std::move_if_noexcept(buffer_.back());
    //     buffer_.pop_back();
    //     not_full_.notify_one();
    //     return res;
    // }

    Value wait_and_pop() noexcept {
        std::unique_lock<std::mutex> lk(mutex_);
        not_empty_.wait(lk, [this] { return !buffer_.empty(); });
        Value res = std::move_if_noexcept(buffer_.back());
        buffer_.pop_back();
        not_full_.notify_one();
        return res;
    }

    inline size_t size() const noexcept {
        std::lock_guard<std::mutex> lk(mutex_);
        return buffer_.size();
    }

    inline bool full() const noexcept {
        std::lock_guard<std::mutex> lk(mutex_);
        return buffer_.full();
    }

    inline bool empty() const noexcept {
        std::lock_guard<std::mutex> lk(mutex_);
        return buffer_.empty();
    }

    ThreadSafeQueue(ThreadSafeQueue const&) = delete;
    ThreadSafeQueue& operator=(ThreadSafeQueue const&) = delete;

    ThreadSafeQueue(ThreadSafeQueue&& other) noexcept
        : buffer_(std::move_if_noexcept(other.buffer_))
        , mutex_(std::move_if_noexcept(other.mutex_))
        , not_full_(std::move_if_noexcept(other.not_full_))
        , not_empty_(std::move_if_noexcept(other.not_empty_)){};

    ThreadSafeQueue& operator=(ThreadSafeQueue&& other) noexcept {
        if (this != &other) { // Self assignment.
            buffer_ = std::move_if_noexcept(other.buffer_);
            mutex_ = std::move_if_noexcept(other.mutex_);
            not_full_ = std::move_if_noexcept(other.not_full_);
            not_empty_ = std::move_if_noexcept(other.not_empty_);
        }
        return *this;
    };

private:
    mutable std::mutex mutex_;
    boost::circular_buffer<T> buffer_;
    std::condition_variable not_full_;
    std::condition_variable not_empty_;
};

#if 0

template <typename T>
class ThreadSafeQueue {
public:
    using Value = T;
    using Self = ThreadSafeQueue;

    explicit ThreadSafeQueue(size_t capacity = UnboundedCapSize) noexcept
        : capacity_(capacity) {
        if (capacity_ > 0) {
            buffer_.reserve(capacity_);
        }
    }

    template <typename... Args, typename = typename std::enable_if<std::is_constructible<Args..., T>::value>::type>
    void emplace(Args&&... args) {
        std::lock_guard<std::mutex> lk(mutex_);
        buffer_.emplace(std::forward<Args>(args)...);
        cond_.notify_all();
    }

    template <typename U>
    void push(U&& val) noexcept {
        std::lock_guard<std::mutex> lk(mutex_);
        buffer_.push_back(std::forward<U>(val));
        cond_.notify_all();
    }

    bool try_pop(Value& res) noexcept {
        std::lock_guard<std::mutex> lk(mutex_);
        if (buffer_.empty())
            return false;
        res = std::move_if_noexcept(buffer_.back()); // move assignment, if no except.
        buffer_.pop_back();
        return true;
    }

    size_t size() const noexcept {
        std::lock_guard<std::mutex> lk(mutex_);
        return buffer_.size();
    }

    bool full() const noexcept { return size() >= capacity_; }

    bool empty() const noexcept { return size() <= 0; }

    Value wait_and_pop() noexcept {
        std::unique_lock<std::mutex> lk(mutex_);
        cond_.wait(lk, [&]() { return !buffer_.empty(); });
        Value res = std::move_if_noexcept(buffer_.back()); // move assignment, if no except.
        buffer_.pop_back();
        return res;
    }

    ThreadSafeQueue(ThreadSafeQueue const&) = delete;
    ThreadSafeQueue& operator=(ThreadSafeQueue const&) = delete;
    ThreadSafeQueue(ThreadSafeQueue&& other) noexcept
        : buffer_(std::move_if_noexcept(other.buffer_))
        , mutex_(std::move_if_noexcept(other.mutex_))
        , cond_(std::move_if_noexcept(other.cond_)){};
    ThreadSafeQueue& operator=(ThreadSafeQueue&& other) noexcept {
        if (this != &other) { // Self assignment.
            buffer_ = std::move_if_noexcept(other.buffer_);
            mutex_ = std::move_if_noexcept(other.mutex_);
            cond_ = std::move_if_noexcept(other.cond_);
        }
        return *this;
    };

private:
    mutable std::mutex mutex_;
    const size_t capacity_;
    std::vector<Value> buffer_;
    std::condition_variable cond_;
};

#endif
