#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_set>
#include <vector>

using task_id_t = size_t;

class ThreadPool {
    private:
        struct Task {
            task_id_t id;
            std::function<void()> func;
        };

    public:
        ThreadPool(size_t threads_count = std::thread::hardware_concurrency());

        ~ThreadPool();

        void quit();

        template<typename F, typename... Args>
        task_id_t add_task(const F& f, Args&&... args) {
            std::lock_guard<std::mutex> tasks_queue_lock(tasks_queue_mtx);
            task_id_t id = next_task_id++;
            tasks_queue.push(Task {
                .id = id,
                .func = std::bind(f, args...),
            });
            task_added_cv.notify_one();
            return id;
        }

        template<typename F, typename... Args>
        task_id_t add_task_now(const F& f, Args&&... args) {
            std::unique_lock<std::mutex> tasks_queue_lock(tasks_queue_mtx);
            task_id_t id = next_task_id++;
            if (tasks_queue.empty() && free_threads > 0) {
                tasks_queue.push(Task {
                    .id = id,
                    .func = std::bind(f, args...),
                });
                task_added_cv.notify_one();
            } else {
                tasks_queue_lock.unlock();
                f(args...);
                std::lock_guard<std::mutex> done_task_ids_lock(done_task_ids_mtx);
                done_task_ids.insert(id);
            }
            return id;
        }

        void wait(task_id_t id);

        void wait_all();

    private:
        void run(size_t thread_num);

        std::queue<Task> tasks_queue;
        std::mutex tasks_queue_mtx;

        std::vector<std::thread> threads;

        std::unordered_set<task_id_t> done_task_ids;
        std::mutex done_task_ids_mtx;

        std::atomic<task_id_t> next_task_id = 0;
        std::atomic<bool> need_to_quit = false;
        std::atomic<size_t> free_threads;

        std::condition_variable task_done_cv;
        std::condition_variable task_added_cv;
};
