#include <iostream>
#include <queue>
#include <thread>

class BTSQ {
private:
  std::queue<int> m_data_queue;
  int m_size;

  std::mutex mtx;
  std::condition_variable not_empty;
  std::condition_variable not_full;

  bool done{false};

public:
  BTSQ(int size) : m_size{size} {}

  void produce(int n) {}
  void consume() {}
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
