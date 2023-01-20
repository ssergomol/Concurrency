#include <atomic>
#include <memory>
#include <thread>
#include <vector>
#include <deque>
#include <future>
#include <type_traits>
#include "joiner.cpp"
#include "stealing_queue.cpp"
#include "threadsafe_queue.cpp"

class ThreadPool {
    const unsigned thread_count;
    typedef function_wrapper task_type;
    std::atomic_bool done;
    threadsafe_queue <task_type> pool_work_queue;
    std::vector<std::unique_ptr<work_stealing_queue> > queues;
    std::vector<std::thread> threads;
    join_threads joiner;
    static thread_local work_stealing_queue *local_work_queue;
    static thread_local unsigned my_index;

    void worker_thread(unsigned my_index_) {
        my_index = my_index_;
        local_work_queue = queues[my_index].get();
        while (!done) {
            run_pending_task();
        }
    }

    bool pop_task_from_local_queue(task_type &task) {
        return local_work_queue && local_work_queue->try_pop(task);
    }

    bool pop_task_from_pool_queue(task_type &task) {
        return pool_work_queue.try_pop(task);
    }

    bool pop_task_from_other_thread_queue(task_type &task) {
        for (unsigned i = 0; i < queues.size(); ++i) {
            unsigned const index = (my_index + i + 1) % queues.size();
            if (queues[index]->try_steal(task)) {
                return true;
            }
        }
        return false;
    }

public:
    explicit ThreadPool(int n_threads) :
            done(false), joiner(threads), thread_count(n_threads) {
        try {
            for (unsigned i = 0; i < thread_count; ++i) {
                queues.push_back(std::make_unique<work_stealing_queue>(
                        ));
            }
            for (unsigned i = 0; i < thread_count; ++i) {
                threads.emplace_back(&ThreadPool::worker_thread, this, i);
            }
        }
        catch (...) {
            done = true;
            throw;
        }
    }

    ~ThreadPool() {
        done = true;
    }

    template<typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type> submit(
            FunctionType f) {
        typedef typename std::result_of<FunctionType()>::type result_type;
        std::packaged_task<result_type()> task(f);
        std::future<result_type> res(task.get_future());
        if (local_work_queue) {
            local_work_queue->push(std::move(task));
        } else {
            pool_work_queue.push(std::move(task));
        }
        return res;
    }

    void run_pending_task() {
        task_type task;
        if (pop_task_from_local_queue(task) ||
            pop_task_from_pool_queue(task) ||
            pop_task_from_other_thread_queue(task)) {
            task();
        } else {
            std::this_thread::yield();
        }
    }
};