#include "engine/threading/thread_pool.hpp"

namespace engine::threading {

ThreadPool::ThreadPool(unsigned int number_of_threads) : _stop(false) {
    for (int thread_id = 0; thread_id < number_of_threads; ++thread_id) {
        this->_threads.emplace_back([this] {
            while (true) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(this->_queue_mutex);

                    this->_condition.wait(lock, [this] {
                        return !this->_tasks.empty() || this->_stop;
                    });

                    if (this->_stop && this->_tasks.empty()) {
                        return;
                    }

                    task = std::move(this->_tasks.front());

                    this->_tasks.pop();
                }

                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(this->_queue_mutex);

        this->_stop = true;
    }

    this->_condition.notify_all();

    for (std::thread &thread : this->_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void ThreadPool::push(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(this->_queue_mutex);

        this->_tasks.emplace(std::move(task));
    }

    this->_condition.notify_one();
}

} // namespace engine::threading
