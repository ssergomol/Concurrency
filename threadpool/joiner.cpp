#include <vector>
#include <thread>

class join_threads {
    std::vector<std::thread> &threads;
public:
    explicit join_threads(std::vector<std::thread> &threads_) :
            threads(threads_) {}

    ~join_threads() {
        for (auto &thread: threads) {
            if (thread.joinable())
                thread.join();
        }
    }
};