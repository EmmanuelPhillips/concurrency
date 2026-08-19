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
    std::unique_lock<std::mutex> lock(
        mtx); // Lock so other threads can't work for now
    cv.wait(lock, [this] { return done; }); // Wait till notify_one() triggers

    while (!data_queue.empty()) { // Check to see if the queue has data
      int p{data_queue.front()};
      data_queue.pop(); // Pop values till the queue is empty
      std::cout << "popped: " << p << '\n';
    }
  };
};

int main() {
  ThreadSafeQueue tsq{};

  std::thread t1(&ThreadSafeQueue::produce, &tsq, 5);
  std::thread t2(&ThreadSafeQueue::consume, &tsq);

  t1.join();
  t2.join();

  return 0;
}
