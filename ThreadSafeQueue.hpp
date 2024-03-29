#include <queue>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

template <typename T>
class ThreadSafeQueue
{
private:
    std::queue<T> queue_;               // �ڲ�����
    mutable std::mutex mutex_;          // ������
    std::condition_variable cond_var_;  // ��������

public:
    // ��Ӳ���
    void enqueue(const T& item)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(item);
        }
        cond_var_.notify_one();  // ֪ͨ�ȴ��е��߳�
    }

    // ���Ӳ���
    T dequeue()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // ʹ��whileѭ���ȴ����Է���ٻ���
        while (queue_.empty())
        {
            cond_var_.wait(lock);  // �ȴ���������
        }
        T item = queue_.front();
        queue_.pop();
        return item;
    }

    // ���Գ��Ӳ�������������
    bool try_dequeue(T& item)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty())
        {
            return false;  // ����Ϊ�գ�����ʧ��
        }
        item = queue_.front();
        queue_.pop();
        return true;  // ���ӳɹ�
    }

    // �������Ƿ�Ϊ��
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    // ��ȡ���д�С
    size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
};
