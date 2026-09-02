#include <iostream>
#include <queue>
#include <thread>

class BTSQ {
private:
  std::queue<int> m_data_queue;
  int m_max_size;

  std::mutex mtx;
  std::condition_variable not_empty;
  std::condition_variable not_full;

  bool done{false};

public:
  BTSQ(int size) : m_max_size{size} {}

  void produce(int n) {
    std::unique_lock<std::mutex> lock(mtx);

    not_full.wait(lock, [this] { return m_data_queue.size() < m_max_size; });
    m_data_queue.push(n);
    std::cout << "produced: " << n << " | queue size: " << m_data_queue.size()
              << '\n';
    not_empty.notify_one();
  }

  void consume() {
    std::unique_lock<std::mutex> lock(mtx);

    not_empty.wait(lock, [this] { return !(m_data_queue.empty()) || done; });
    int n{m_data_queue.front()};
    m_data_queue.pop();
    std::cout << "consumed: " << n << " | queue size: " << m_data_queue.size();
    not_full.notify_one();
  }
};

int main() {
  BTSQ data_queue{5};

  std::thread t1(&BTSQ::produce, &data_queue, 5);
  std::thread t2(&BTSQ::consume, &data_queue);

  t1.join();
  t2.join();

  std::cout << '\n';
  return 0;
}
