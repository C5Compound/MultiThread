/**
 *  信号量的实现见 《 条件变量实现信号量 》
 *
 *  单生产者单消费者问题
 *  多生产者消费者问题
 *  带transmitter的问题
 */

// 1.单个生产者消费者问题
#define BUF_SIZE 12
int Process()
{
    std::mutex lock;
    karel::semaphore full(0), empty(BUF_SIZE);
    int producer_ptr = 0, consumer_ptr = 0, buf[BUF_SIZE];
    thread producer([&]() {
        while (true) {
            empty.wait();
            lock.lock();
            // 生产过程
            printf("produce %d\n", producer_ptr);
            buf[producer_ptr] = producer_ptr;
            producer_ptr = (1 + producer_ptr) % BUF_SIZE;
            lock.unlock();
            full.signal();
        }
    });
    thread consumer([&]() {
        while (true) {
            full.wait();
            lock.lock();
            // 消费过程
            printf("consume %d\n", buf[consumer_ptr]);
            consumer_ptr = (1 + consumer_ptr) % BUF_SIZE;
            lock.unlock();
            empty.signal();
        }
    });
    producer.join();
    consumer.join();
    return 0;
}

// 2.多生产者多消费者问题
// 1所示模型也可以解决多生产者多消费问题


// 3.带transmitter的生产者消费者问题
#define BUF_SIZE1 12
#define BUF_SIZE2 6
int Process()
{
    mutex lock1, lock2;
    int buff1[BUF_SIZE1], buff2[BUF_SIZE2];
    int ip1 = 0, ic1 = 0, ip2 = 0, ic2 = 0;
    karel::semaphore empty1(BUF_SIZE1), full1(0), empty2(BUF_SIZE2), full2(0);
    thread producer([&]() {
        do {
            std::this_thread::sleep_for(chrono::milliseconds(500));
            empty1.wait();
            lock1.lock();
            // 生产
            buff1[ip1] = ip1;
            printf("生产了%d\n", ip1);
            ip1 = (ip1 + 1) % BUF_SIZE1;
            lock1.unlock();
            full1.signal();
        } while (true);
    });
    thread transmitter([&]() {
        do {
            full1.wait();
            empty2.wait();
            lock1.lock();
            lock2.lock();
            // 传输
            buff2[ip2] = buff1[ic1];
            printf("传输了%d\n", buff2[ip2]);
            ic1 = (1 + ic1) % BUF_SIZE1;
            ip2 = (1 + ip2) % BUF_SIZE2;
            lock2.unlock();
            lock1.unlock();
            full2.signal();
            empty1.signal();
        } while (true);
    });
    thread consumer([&]() {
        do {
            std::this_thread::sleep_for(chrono::milliseconds(500));
            full2.wait();
            lock2.lock();
            // 消费
            printf("消费了%d\n", buff2[ic2]);
            ic2 = (ic2 + 1) % BUF_SIZE2;
            lock2.unlock();
            empty2.signal();
        } while (true);
    });
    producer.join();
    transmitter.join();
    consumer.join();
    return 0;
}