#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

class ThreadSafeQueue {
private:
  const int m_max_size;
  std::queue<int> m_data_queue;

  std::mutex mtx;
  std::condition_variable not_full;
  std::condition_variable not_empty;

  bool done{false};

public:
  ThreadSafeQueue(int max_size) : m_max_size(max_size) {}
  void produce(int n) {}
  void consume() {}
};

int main() {
  std::thread p1;
  std::thread p2;
  std::thread c1;
  std::thread c2;

  p1.join();
  p2.join();
  c1.join();
  c2.join();
  return 0;
}
