#include <queue>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

template <typename T>
class ThreadSafeQueue
{
private:
    std::queue<T> queue_;               // 内部队列
    mutable std::mutex mutex_;          // 互斥锁
    std::condition_variable cond_var_;  // 条件变量

public:
    // 入队操作
    void enqueue(const T& item)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(item);
        }
        cond_var_.notify_one();  // 通知等待中的线程
    }

    // 出队操作
    T dequeue()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // 使用while循环等待，以防虚假唤醒
        while (queue_.empty())
        {
            cond_var_.wait(lock);  // 等待条件变量
        }
        T item = queue_.front();
        queue_.pop();
        return item;
    }

    // 尝试出队操作（非阻塞）
    bool try_dequeue(T& item)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty())
        {
            return false;  // 队列为空，出队失败
        }
        item = queue_.front();
        queue_.pop();
        return true;  // 出队成功
    }

    // 检查队列是否为空
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    // 获取队列大小
    size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
};
