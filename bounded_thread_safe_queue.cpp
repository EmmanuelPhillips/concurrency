#include <iostream>
#include <queue>
#include <thread>

class BTSQ {
private:
  std::queue<int> queue_;
  int max_size;

public:
};

int main() {
  std::mutex mtx;
  std::condition_variable cv;

  std::thread t1;
  std::thread t2;

  t1.join();
  t2.join();

  BTSQ btsq{};

  std::cout << '\n';
  return 0;
}
