#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

class ThreadSafeQueue {
private:
  std::queue<int> data_queue{};

  std::mutex mtx;
  std::condition_variable cv;
  bool done{false};

public:
  void produce(int n) {
    std::unique_lock<std::mutex> lock(
        mtx); // Lock so other threads can't work for now
    for (int i{1}; i <= n; ++i) {
      data_queue.push(i); // Produce integers to queue
      std::cout << "produced: " << i << '\n';
    }
    done =
        true; // Set done to true so consume knows how many integers there are
    cv.notify_one(); // Notify other thread that it's being unlocked
  }

  void consume() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return; });

    while (!data_queue.empty() && done) {
      int p{data_queue.front()};
      data_queue.pop();
      std::cout << "popped: " << p << '\n';
    }
  };
};

int main() {
  ThreadSafeQueue tsq{};

  std::thread t1(&ThreadSafeQueue::produce, 5);
  std::thread t2(&ThreadSafeQueue::consume);

  t1.join();
  t2.join();

  return 0;
}
