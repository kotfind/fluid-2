#include "ThreadPool.hpp"

ThreadPool::ThreadPool(size_t threads_count) {
    threads.reserve(threads_count);
    for (size_t thread_num = 0; thread_num < threads_count; ++thread_num) {
        threads.emplace_back(&ThreadPool::run, this, thread_num);
    }
}

ThreadPool::~ThreadPool() {
    quit();
    for (auto& thread : threads) {
        thread.join();
    }
}

void ThreadPool::quit() {
    need_to_quit = true;
    task_added_cv.notify_all();
}

void ThreadPool::wait(task_id_t id) {
    std::unique_lock<std::mutex> done_task_ids_lock(done_task_ids_mtx);
    task_done_cv.wait(done_task_ids_lock, [this, id]{
        return done_task_ids.contains(id);
    });
}

void ThreadPool::wait_all() {
    std::unique_lock<std::mutex> done_task_ids_lock(done_task_ids_mtx);
    task_done_cv.wait(done_task_ids_lock, [this]{
        std::lock_guard<std::mutex> tasks_queue_lock(tasks_queue_mtx);
        return done_task_ids.size() == next_task_id;
    });
}

void ThreadPool::run(size_t thread_num) {
    while (!need_to_quit) {
        std::unique_lock<std::mutex> tasks_queue_lock(tasks_queue_mtx);
        task_added_cv.wait(tasks_queue_lock, [this]{
            return need_to_quit || !tasks_queue.empty();
        });

        if (need_to_quit) break;

        auto task = std::move(tasks_queue.front());
        tasks_queue.pop();
        tasks_queue_lock.unlock();

        task.func();

        std::lock_guard<std::mutex> done_task_ids_lock(done_task_ids_mtx);
        done_task_ids.insert(task.id);

        task_done_cv.notify_one();
    }
}
