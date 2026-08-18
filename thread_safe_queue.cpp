/*
 * task: thread-safe queue
 *
 * build a queue that multiple threads can safely push to and pop from
 * at the same time.
 *
 * requirements:
 * - a producer thread pushes integer values into the queue
 * - a consumer thread pops values out of the queue
 * - if the queue is empty when the consumer tries to pop, it should block
 *   (wait efficiently) rather than busy-loop checking repeatedly
 * - when the producer pushes a new value, the waiting consumer should wake
 *   up and be able to process it
 * - the underlying queue data must never be accessed by two threads at
 *   the same time
 */

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

class ThreadSafeQueue {
private:
  std::queue<int> queue_{};
  std::mutex mtx;
  std::condition_variable cv;

public:
  void push(int n) {
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << "pushing: " << n;
    queue_.push(n);
    cv.notify_one();
  }

  void pop() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this]() { return !queue_.empty(); });
    std::cout << "popping: " << queue_.front();
    queue_.pop();
  }
};

int main() {
  ThreadSafeQueue my_queue{};

  std::thread t1(&ThreadSafeQueue::push, &my_queue, 1);
  std::thread t2(&ThreadSafeQueue::pop, &my_queue);

  t1.join();
  t2.join();

  return 0;
}
