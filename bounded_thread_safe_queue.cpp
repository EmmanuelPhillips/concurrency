#include <iostream>
#include <queue>
#include <thread>

class BTSQ {
private:
  std::queue<int> data_queue;
  int max_size;

  std::mutex mtx;
  std::condition_variable cv;

  bool done{false};

public:
  void produce(int n) {
    std::unique_lock<std::mutex> lock(mtx);
    if (data_queue.size() >= max_size) {
      return;
    } else {
      data_queue.push(n);
      return;
    }
    cv.notify_one();
  }
};

int main() {
  BTSQ btsq{};

  std::thread t1(&BTSQ::produce, &btsq, 5);
  std::thread t2;

  t1.join();
  t2.join();

  std::cout << '\n';
  return 0;
}
