#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

template<typename T>
class threadsafe_queue {
public:
    threadsafe_queue() = default;                                     // default constructor
    threadsafe_queue(const threadsafe_queue&) = delete;               // copy constructor
    threadsafe_queue& operator=(const threadsafe_queue&) = delete;    // copy assignment
    threadsafe_queue(threadsafe_queue&&) = delete;                    // move constructor
    threadsafe_queue& operator=(threadsafe_queue &&) = delete;        // move assignment

    void push(T const& data) {
        std::unique_lock<std::mutex> lock(_mutex);
        _queue.push(data);
        lock.unlock();
        _condition.notify_one();
    }

    bool try_pop(T& value) {
        std::unique_lock<std::mutex> lock(_mutex);

        if (_queue.empty()) {
            return false;
        }

        value = std::move(_queue.front());
        _queue.pop();
        return true;
    }

private:
    std::queue<T> _queue;
    std::mutex _mutex;
    std::condition_variable _condition;
};