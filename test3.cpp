//#include <iostream>
//#include <thread>
//#include "ThreadSafeQueue.hpp"  // 假设ThreadSafeQueue定义在ThreadSafeQueue.h中
//
//// 生产者函数
//void producer(ThreadSafeQueue<int>& q)
//{
//    for (int i = 0; i < 10; ++i)
//    {
//        std::cout << "Producing: " << i << std::endl;
//        q.enqueue(i);
//        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 等待100毫秒
//    }
//}
//
//// 消费者函数
//void consumer(ThreadSafeQueue<int>& q)
//{
//    for (int i = 0; i < 10; ++i)
//    {
//        int item;
//        if (q.try_dequeue(item))
//        {
//            std::cout << "Consuming: " << item << std::endl;
//        }
//        else
//        {
//            std::cout << "Queue is empty" << std::endl;
//        }
//        std::this_thread::sleep_for(std::chrono::milliseconds(150));  // 等待150毫秒
//    }
//}
//
//int main()
//{
//    ThreadSafeQueue<int> q;
//
//    // 创建生产者线程和消费者线程
//    std::thread producer_thread(producer, std::ref(q));
//    std::thread consumer_thread(consumer, std::ref(q));
//
//    // 等待线程结束
//    producer_thread.join();
//    consumer_thread.join();
//
//    return 0;
//}
