#pragma once

#include "engine/core/Core.hpp"
#include "pch.hpp"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace Galaxy {
const size_t maxThreads = std::thread::hardware_concurrency();

const size_t resourcePoolSize = maxThreads * 0.4;

class ThreadPool {
public:
    ThreadPool(size_t nbThreads = std::thread::hardware_concurrency())
    {
        GLX_CORE_ASSERT(nbThreads >= 1, "Thread pool size is 0");
        for (int i = 0; i < nbThreads; i++) {
            m_threads.emplace_back([this] {
                while (true) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(m_queueMutex);

                        m_cv.wait(lock, [this] { return !m_tasks.empty() || m_stop; });

                        if (m_stop && m_tasks.empty()) {
                            return;
                        }

                        task = std::move(m_tasks.front());
                        m_tasks.pop();
                    }

                    task();
                }
            });
        }
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_stop = true;
        }

        m_cv.notify_all();

        for (auto& thread : m_threads) {
            thread.join();
        }
    }

    void enqueue(std::function<void()> task)
    {
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_tasks.emplace(move(task));
        }
        m_cv.notify_one();
    }

private:
    std::vector<std::thread> m_threads;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_queueMutex;

    std::condition_variable m_cv;

    bool m_stop = false;
};
} // namespace Galaxy
