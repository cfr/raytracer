#pragma once

#include <algorithm>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <concepts>
#include <stop_token>
#include <memory>
#include <stdexcept>
#include <utility>

namespace raytracer {

class ThreadPool {
 public:
    explicit ThreadPool(size_t size) {
        size = std::max(1ul, size);
        workers_.reserve(size);

        for (size_t i = 0; i < size; ++i) {
            workers_.emplace_back([this](std::stop_token st) {
                loop(st);
            });
        }
    }

    ~ThreadPool() {
        for (auto& worker : workers_) {
            worker.request_stop();
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    template <typename F, typename... Args>
    requires std::invocable<F, Args...>
    auto submit(F&& f, Args&&... args) {
        using ReturnType = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<ReturnType> result = task->get_future();

        {
            std::unique_lock<std::mutex> lock(qmut_);
            tasks_.push([task]() { (*task)(); });
        }

        cv_.notify_one();
        return result;
    }

 private:
    void loop(std::stop_token st) {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(qmut_);

                cv_.wait(lock, st, [this, &st] {
                    return !tasks_.empty() || st.stop_requested();
                });

                if (st.stop_requested()) { return; }

                task = std::move(tasks_.front());
                tasks_.pop();
            }

            task();
        }
    }

    std::mutex qmut_;
    std::condition_variable_any cv_;
    std::queue<std::function<void()>> tasks_;
    std::vector<std::jthread> workers_;
};

}  // namespace raytracer
