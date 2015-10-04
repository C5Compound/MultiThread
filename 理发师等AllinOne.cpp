#include <atomic>
#include <condition_variable>
#include <mutex>

#include <thread>
#include <iostream>

// 二进制信号量与mutex的区别是：
// ownership的区别，mutex只能由lock的线程释放，binary semaphore可以在其他线程signal

namespace karel {
    class semaphore {
    public:
        semaphore(int ix = 0) : count(ix) {}
        void wait() {
            std::unique_lock<std::mutex> lock(mtx);
            if (--count < 0) {
                cdv.wait(lock);
            }
        }
        void signal() {
            std::unique_lock<std::mutex> lock(mtx);
            if (++count <= 0) {
                cdv.notify_one();
            }
        }
    private:
        int count;
        std::condition_variable cdv;
        std::mutex mtx;
    };
}

// 理发师问题

// 等待服务的客户数
karel::semaphore _customers(0);
// 等待客户的理发师数
karel::semaphore _barbers(0);
// 互斥锁
std::mutex count_lock;
// 计数值
#define CHAIRS 5
int waitings = 0;

void barbers() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
        _customers.wait();
        count_lock.lock();
        --waitings;
        count_lock.unlock();
        _barbers.signal();
        std::cout << "cut hair for" << std::endl;
    }
}

void customer() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        count_lock.lock();
        if (waitings < CHAIRS) {
            ++waitings;
            _customers.signal();
            count_lock.unlock();
            _barbers.wait();
            std::cout << "wait for hair cut" << std::endl;
        } else {
            std::cout << "directly leave, too many people" << std::endl;
            count_lock.unlock();
        }
    }
}

// 生产者消费者问题
// 只有一个缓冲区的情况

int i = -1;
karel::semaphore full(0), empty(1);

void consumer() {
    while (true) {
        full.wait();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::cout << "consumer " << i << std::endl;
        empty.signal();
    }
}

void producer() {
    while (true) {
        empty.wait();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::cout << "producer " << ++i << std::endl;
        full.signal();
    }
}

// 读者写着问题

// 读者优先的情况
#ifdef READER_FIRST 
std::mutex readers_count_lock;
karel::semaphore writers(1);
int readers_count = 0, buf = 0;

void write() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        writers.wait();
        ++buf;
        std::cout << "increase buf to: " << buf << std::endl;
        writers.signal();
    }
}

void read() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        readers_count_lock.lock();
        if (++readers_count == 1) {
            writers.wait();
        }
        readers_count_lock.unlock();
        std::cout << "read buf, the value of buf is: " << buf << std::endl;
        readers_count_lock.lock();
        if (--readers_count == 0) {
            writers.signal();
        }
        readers_count_lock.unlock();
    }
}
#endif

// 公平竞争的情况
#ifdef JUSTICE
std::mutex readers_count_lock;
karel::semaphore queue(1), writers(1);
int readers_count = 0, buf = 0;

void write() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        queue.wait();
        writers.wait();
        ++buf;
        std::cout << "increase buf to: " << buf << std::endl;
        writers.signal();
        queue.signal();
    }
}

void read() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        queue.wait();
        readers_count_lock.lock();
        if (++readers_count == 1) {
            writers.wait();
        }
        readers_count_lock.unlock();
        queue.signal();
        std::cout << "read buf, the value of buf is: " << buf << std::endl;
        readers_count_lock.lock();
        if (--readers_count == 0) {
            writers.signal();
        }
        readers_count_lock.unlock();
    }
}
#endif

// 写者优先的情况
#ifdef WRITE_FIRST
std::mutex readers_count_lock, writers_count_lock;
karel::semaphore queue(1), writers(1);
int readers_count = 0, writers_count = 0, buf = 0;

void write() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        writers_count_lock.lock();
        if (++writers_count == 1) {
            queue.wait();
        }
        writers_count_lock.unlock();
        writers.wait();
        ++buf;
        std::cout << "increase buf to: " << buf << std::endl;
        writers.signal();
        writers_count_lock.lock();
        if (--writers_count == 0) {
            queue.signal();
        }
        writers_count_lock.unlock();
    }
}

void read() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        queue.wait();
        readers_count_lock.lock();
        if (++readers_count == 1) {
            writers.wait();
        }
        readers_count_lock.unlock();
        queue.signal();
        std::cout << "read buf, the value of buf is: " << buf << std::endl;
        readers_count_lock.lock();
        if (--readers_count == 0) {
            writers.signal();
        }
        readers_count_lock.unlock();
    }
}
#endif


int main() {
    std::thread t1(barbers);
    std::thread t2(customer);
    t1.join();
    t2.join();
    return 0;
}
