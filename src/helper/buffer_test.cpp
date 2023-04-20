#include <gtest/gtest.h>

#include "./buffer.hpp"
#include "./literal.hpp"

using String = std::string;

TEST(BUFFER, BoundedQueue) {
    ThreadSafeQueue<String> queue(4);
    queue.wait_and_push("0");
    queue.wait_and_push("1");
    queue.wait_and_push("2");
    queue.wait_and_push("3");
    EXPECT_TRUE(queue.full());
}

TEST(RingBuffer, Captity) {
    RingBuffer<String> buffer(2);
    EXPECT_TRUE(buffer.try_push("0"));
    EXPECT_TRUE(buffer.try_push("1"));
    EXPECT_TRUE(!buffer.try_push("2"));

    String val;
    EXPECT_TRUE(buffer.try_pop(val));
    EXPECT_EQ(val, "0"_str);
    EXPECT_TRUE(buffer.try_pop(val));
    EXPECT_EQ(val, "1"_str);
    EXPECT_TRUE(!buffer.try_pop(val));
}
