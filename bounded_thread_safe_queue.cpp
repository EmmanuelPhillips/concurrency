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
  BTSQ(int size) : max_size{size} {}
  void produce(int n) {
    std::unique_lock<std::mutex> lock(mtx);
    if (data_queue.size() >= max_size) {
      cv.notify_one();
      return;
    } else {
      data_queue.push(n);
    }
    cv.notify_one();
    return;
  }
};

int main() {
  BTSQ btsq{5};

  std::thread t1(&BTSQ::produce, &btsq, 5);
  std::thread t2;

  t1.join();
  t2.join();

  std::cout << '\n';
  return 0;
}
