/**
 *  读者写者问题分为三类
 *  读者优先
 *  写者优先
 *  先到先服务
 */
// 1.读者优先
int Process()
{
    karel::semaphore sem_read(1), writemutex(1);
    int readcount = 0, buf = 0;
    thread reader([&]() {
        while (true) {
            sem_read.wait();
            if (++readcount == 1) {
                writemutex.wait();
            }
            sem_read.signal();
            // 读操作
            printf("reader process: %d\n", buf);
            sem_read.wait();
            if (--readcount == 0) {
                writemutex.signal();
            }
            sem_read.signal();
        }
    });
    thread writer([&]() {
        while (true) {
            writemutex.wait();
            // 写过程
            buf += 1;
            printf("writer process: %d\n", buf);
            writemutex.signal();
        }
    });
    reader.join();
    writer.join();
    return 0;
}

// 2.写者优先
int Process()
{
    karel::semaphore sem_read(1), sem_write(1), queue(1), writemutex(1);
    int readcount = 0, writecount = 0, buf = 0;
    thread reader([&]() {
        while (true) {
            queue.wait();
            sem_read.wait();
            if (++readcount == 1) {
                writemutex.wait();
            }
            sem_read.signal();
            queue.signal();
            // 读操作
            printf("reader process: %d\n", buf);
            sem_read.wait();
            if (--readcount == 0) {
                writemutex.signal();
            }
            sem_read.signal();
        }
    });
    thread writer([&]() {
        while (true) {
            sem_write.wait();
            if (++writecount == 1) {
                queue.wait();
            }
            sem_write.signal();
            writemutex.wait();
            // 写过程
            buf += 1;
            printf("writer process: %d\n", buf);
            writemutex.signal();
            sem_write.wait();
            if (--writecount == 0) {
                queue.signal();
            }
            sem_write.signal();
        }
    });
    reader.join();
    writer.join();
    return 0;
}

// 3.公平竞争的情况
int Process()
{
    karel::semaphore sem_read(1), writemutex(1), queue(1);
    int readcount = 0, buf = 0;
    thread reader([&]() {
        while (true) {
            queue.wait();
            sem_read.wait();
            if (++readcount == 1) {
                writemutex.wait();
            }
            sem_read.signal();
            queue.signal();
            // 读操作
            printf("reader process: %d\n", buf);
            sem_read.wait();
            if (--readcount == 0) {
                writemutex.signal();
            }
            sem_read.signal();
        }
    });
    thread writer([&]() {
        while (true) {
            queue.wait();
            writemutex.wait();
            // 写过程
            buf += 1;
            printf("writer process: %d\n", buf);
            writemutex.signal();
            queue.signal();
        }
    });
    reader.join();
    writer.join();
    return 0;
}