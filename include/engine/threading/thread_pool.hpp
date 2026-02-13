#pragma once

#include <queue>
#include <mutex>
#include <functional>
#include <condition_variable>

namespace engine::threading {

class ThreadPool {
  public:
    ThreadPool(unsigned int number_of_threads = std::thread::hardware_concurrency());

    ~ThreadPool();

    void push(std::function<void()> task);

  private:
    // static inline constexpr int _NUMBER_OF_THREADS = std::thread::hardware_concurrency();

    std::vector<std::thread> _threads;

    std::queue<std::function<void()>> _tasks;

    std::mutex _queue_mutex;

    std::condition_variable _condition;

    bool _stop;
};

} // namespace engine::threading
