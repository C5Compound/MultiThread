/**
 *  用C++11实现semaphore，实现P和V操作
 */
#include <condition_variable>
#include <atomic>
#include <mutex>

namespace karel
{
    class semaphore
    {
    public:
        semaphore(int val = 0) : count(val) {};
        void wait() {
            // 这种锁在作用域结束时会自动释放
            std::unique_lock<std::mutex> lock(mtx);
            if (--count < 0) {
                // 当前线程挂起
                cdv.wait();
            }
        }
        void signal() {
            std::unique_lock<std::mutex> lock(mtx);
            if (++count <= 0) {
                // 唤醒一个线程
                cdv.notify_one();
            }
        }
    private:
        int count;
        std::mutex mtx;
        std::condition_variable cdv;
    };
}