

#include <iostream>

#include "future_wrapper/executor.hpp"
#include "future_wrapper/future.hpp"
#include "future_wrapper/promise.hpp"

void printHelloNative(SharedStateBase& sharedState) {
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    std::cout << "[" << std::this_thread::get_id() << "] "
              << "Hello: ("
              << ")" << std::endl;
}

struct Foo {
    String value;
    int x;
    double y;
    char z[128];
    char* ptr;

    friend std::ostream& operator<<(std::ostream& os, const Foo& foo) noexcept {
        return os << "Foo: \n"
                  << "  x: " << foo.x << "\n"
                  << "  y: " << foo.y << "\n"
                  << "  z: " << foo.z << "\n"
                  << "  str: " << foo.value << "\n"
                  << "  ptr: " << foo.ptr << std::endl;
    }
};

void printHelloFoo(Foo&& foo) {
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    std::cout << "[" << std::this_thread::get_id() << "] "
              << "Hello: (" << foo << ")" << std::endl;
}

void printHelloString(String&& value) {
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    std::cout << "[" << std::this_thread::get_id() << "] "
              << "Hello: " << value << std::endl;
}

int main() {
    ThreadExecutor threadExecutor(2);

    Foo foo;
    foo.value = "foo.value";

    String some_data = "some data";
    foo.ptr = const_cast<char*>(some_data.c_str());
    // foo.ptr = nullptr;

    foo.x = 10;
    foo.y = 20.0f;

    sprintf(foo.z, "zzzzzzzzz");

    Promise<String> p1;
    auto f1 = p1.getFuture();

    Promise<Foo> p2;
    auto f2 = p2.getFuture();

    f1.via(&threadExecutor);
    f1.thenValue(printHelloString);
    p1.setValue("Kitty1");
    std::move(f1).get(); // async.

    f2.via(&threadExecutor);
    f2.thenValue(printHelloFoo);
    p2.setValue(std::move(foo));
    std::move(f2).get(); // async.

    std::cout << "[" << std::this_thread::get_id() << "] "
              << "Main Thread say: Bye~" << std::endl;

    return 0;
}
