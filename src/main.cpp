

#include <iostream>

#include "future_wrapper/executor.hpp"
#include "future_wrapper/future.hpp"
#include "future_wrapper/promise.hpp"

void printHello(Value&& value) {
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    std::cout << "[" << std::this_thread::get_id() << "] "
              << "Hello: " << value << std::endl;
}

int main() {

    std::unique_ptr<Executor> pExecutor = std::make_unique<ThreadExecutor>(1);
    auto* executor = pExecutor.get();

    Promise<String> p1;
    auto f1 = p1.getFuture();

    Promise<String> p2;
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
