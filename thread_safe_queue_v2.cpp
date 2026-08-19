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
    for (int i{1}; i <= n; ++i) {
      {
        std::unique_lock<std::mutex> lock(mtx);
        data_queue.push(i); // Produce integers to queue
        std::cout << "produced: " << i << '\n';
      }
      cv.notify_one();
    } // Scope locks and unlocks between each integer produced
    {
      std::unique_lock<std::mutex> lock(mtx);
      done =
          true; // Set done to true so consume knows how many integers there are
    }
    cv.notify_one(); // Wake the other thread
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
  }

  void print_queue() {
    std::unique_lock<std::mutex> lock(mtx);
    if (data_queue.empty()) {
      std::cout << "queue is empty.\n";
      return;
    }
    std::cout << "current queue: ";
    auto copy{data_queue}; // Create copy to iterate over
    while (!copy.empty()) {
      std::cout << copy.front() << ", ";
      copy.pop();
    }
    std::cout << '\n';
  }
};

int main() {
  ThreadSafeQueue tsq{};

  std::thread t1(&ThreadSafeQueue::produce, &tsq, 5);
  std::thread t2(&ThreadSafeQueue::consume, &tsq);

  tsq.print_queue();
  t1.join();
  tsq.print_queue();
  t2.join();
  tsq.print_queue();

  return 0;
}
