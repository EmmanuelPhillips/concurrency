#include <iostream>
#include <mutex>
#include <thread>

std::mutex mtx;

void increment(int &c) {
  std::lock_guard<std::mutex> lock(mtx); // lock scope (no race condition now)
  for (int i{0}; i < 100000; ++i) {
    ++c;
  }
}

int main() {
  int c{0};

  std::thread t1(increment, std::ref(c));
  std::thread t2(increment, std::ref(c));

  t1.join();
  t2.join();

  std::cout << "value of c: " << c << '\n';

  return 0;
}
