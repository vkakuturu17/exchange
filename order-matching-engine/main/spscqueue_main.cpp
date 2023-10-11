#include <rigtorp/SPSCQueue.h>
#include <iostream>
#include <thread>

#define QUEUE_SIZE 1024

typedef rigtorp::SPSCQueue<int> queue;

queue q(QUEUE_SIZE);

int main() {
  std::thread t1([&] {
    for (int i = 0; i < 10; i++) {
      q.push(i);
    }
    std::this_thread::yield();
  });
  std::thread t3([&] {
    for (int i = 10; i < 20; i++) {
      q.push(i);
    }
    std::this_thread::yield();
  });
  std::thread t2([&] {
    int* i;
    while ((i = q.front()) != nullptr) {
      std::cout << *i << std::endl;
      q.pop();
    }
    std::this_thread::yield();
  });
  t1.join();
  t3.join();
  t2.join();
}