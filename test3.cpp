//#include <iostream>
//#include <thread>
//#include "ThreadSafeQueue.hpp"  // ����ThreadSafeQueue������ThreadSafeQueue.h��
//
//// �����ߺ���
//void producer(ThreadSafeQueue<int>& q)
//{
//    for (int i = 0; i < 10; ++i)
//    {
//        std::cout << "Producing: " << i << std::endl;
//        q.enqueue(i);
//        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // �ȴ�100����
//    }
//}
//
//// �����ߺ���
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
//        std::this_thread::sleep_for(std::chrono::milliseconds(150));  // �ȴ�150����
//    }
//}
//
//int main()
//{
//    ThreadSafeQueue<int> q;
//
//    // �����������̺߳��������߳�
//    std::thread producer_thread(producer, std::ref(q));
//    std::thread consumer_thread(consumer, std::ref(q));
//
//    // �ȴ��߳̽���
//    producer_thread.join();
//    consumer_thread.join();
//
//    return 0;
//}
