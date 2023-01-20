#include <iostream>
#include "thread_pool.cpp"

void task() {
    std::cout << 1 << std::endl;
}

int main() {
    ThreadPool pool(5);

//    for (int i = 0; i < 20; i++) {
//        pool.submit(std::function<void>(task));
//    }




    return 0;
}
