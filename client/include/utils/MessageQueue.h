#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <future>
#include <queue>
#include <vector>
#include <iostream>
#include <thread>
#include <utility>

// 定义一个任务类型
using Task = std::function<void()>;

struct DelayedMessage {
    Task task;
    std::chrono::steady_clock::time_point execute_time;

    bool operator>(const DelayedMessage& other) const {
        return execute_time > other.execute_time;
    }
};

class MessageQueue {
public:
    void Post(Task task) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_tasks.push({std::move(task), std::chrono::steady_clock::now()});
        }
        m_cond.notify_one();
    }

    void PostDelayed(Task task, long delay_ms) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto execute_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay_ms);
            m_delayedTasks.push({std::move(task), execute_time});
        }
        m_cond.notify_one();
    }

    void Run() {
        while (!m_stop) {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_delayedTasks.empty() && m_tasks.empty()) {
                m_cond.wait(lock);
            } else if (!m_delayedTasks.empty()) {
                auto& next = m_delayedTasks.top();
                if (std::chrono::steady_clock::now() >= next.execute_time) {
                    auto task = std::move(next.task);
                    m_delayedTasks.pop();
                    lock.unlock();
                    task();
                    continue;
                } else {
                    m_cond.wait_until(lock, next.execute_time);
                }
            }

            if (!m_tasks.empty()) {
                auto task = std::move(m_tasks.front().task);
                m_tasks.pop();
                lock.unlock();
                task();
            }
        }
    }

    void Stop() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stop = true;
        }
        m_cond.notify_all();
    }

private:
    std::priority_queue<DelayedMessage, std::vector<DelayedMessage>, std::greater<DelayedMessage>> m_delayedTasks;
    std::queue<DelayedMessage> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    bool m_stop{false};
};

//实现线程间互相发送消息
// #include <iostream>
// #include <thread>
// #include <memory>

// // 假设 MessageQueue 类的定义保持不变

// void ThreadFunction(MessageQueue& ownQueue, MessageQueue& otherQueue, const std::string& threadName) {
//     // 使当前线程定期向另一个线程发送消息
//     int counter = 0;
//     while (counter < 5) {
//         ownQueue.Post([&otherQueue, counter, threadName]() {
//             std::cout << "Thread " << threadName << " sending message " << counter << std::endl;
//             otherQueue.Post([threadName, counter]() {
//                 std::cout << "Thread " << threadName << " received message " << counter << std::endl;
//             });
//         });
//         std::this_thread::sleep_for(std::chrono::seconds(1));
//         ++counter;
//     }
//     ownQueue.Stop(); // 停止当前线程的消息队列处理循环
// }

// int main() {
//     // 创建两个消息队列，每个线程一个
//     MessageQueue queueA, queueB;

//     // 创建并启动线程A和线程B
//     std::thread threadA(ThreadFunction, std::ref(queueA), std::ref(queueB), "A");
//     std::thread threadB(ThreadFunction, std::ref(queueB), std::ref(queueA), "B");

//     // 等待线程完成
//     threadA.join();
//     threadB.join();

//     return 0;
// }




class TaskQueue {
public:
    TaskQueue() : stop_flag_(false) {
        worker_thread_ = std::thread([this] { this->ProcessTasks(); });
    }

    ~TaskQueue() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_flag_ = true;
        }
        cond_var_.notify_one();
        worker_thread_.join();
    }

    template<typename Func, typename... Args>
    auto Invoke(Func&& func, Args&&... args) -> std::future<decltype(func(args...))> {
        using ReturnType = decltype(func(args...));

        auto task_ptr = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
        );

        auto res = task_ptr->get_future();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stop_flag_) throw std::runtime_error("Invoke on stopped TaskQueue");
            tasks_.emplace([task_ptr](){ (*task_ptr)(); });
        }
        cond_var_.notify_one();
        return res;
    }

private:
    std::thread worker_thread_;
    std::mutex mutex_;
    std::condition_variable cond_var_;
    bool stop_flag_;
    std::queue<std::function<void()>> tasks_;

    void ProcessTasks() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cond_var_.wait(lock, [this] { return stop_flag_ || !tasks_.empty(); });
                if (stop_flag_ && tasks_.empty()) return;
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            task();
        }
    }
};




//使用taskqueue执行任务
// TaskQueue transport_thread1_;
// TaskQueue transport_thread2_;

// // 示例任务
// auto task1 = []() {
//     // 模拟复杂操作
//     std::this_thread::sleep_for(std::chrono::seconds(1));
//     return "Task 1 completed";
// };

// auto task2 = []() {
//     // 模拟另一个复杂操作
//     std::this_thread::sleep_for(std::chrono::seconds(1));
//     return "Task 2 completed";
// };

// int main() {
//     // 在transport_thread1_上执行task1，并获取返回值
//     auto result1 = transport_thread1_.Invoke(task1);
//     // 在transport_thread2_上执行task2，并获取返回值
//     auto result2 = transport_thread2_.Invoke(task2);

//     // 等待任务完成并输出结果
//     std::cout << result1.get() << std::endl;
//     std::cout << result2.get() << std::endl;

//     return 0;
// }
