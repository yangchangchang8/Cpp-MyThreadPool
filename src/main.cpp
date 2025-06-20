#include "myThreadPool.h"
#include <functional>
#include <iostream>
#include <mutex>

std::mutex global_cout_mutex;

void addcal(int x, int y) {
    std::lock_guard<std::mutex> lock(global_cout_mutex);
    std::cout << x << " + " << y << " = " << x + y << std::endl;
}


int main() {
    myThreadPool pool(4);
    for (int i = 0; i < 1100; ++i) {
        auto f = std::bind(addcal, i, i*2);
        pool.addTask(f);
    }
    // 暂停主线程，等待子线程执行完
    getchar();
    return 0;
}