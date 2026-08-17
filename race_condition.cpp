#include <atomic>
#include <iostream>
#include <thread>

std::atomic<int> c{0}; // now using std::atomic instead of std::mutex. this is
                       // lighter weight and doesnt require mutex

void increment(int &c) {
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
