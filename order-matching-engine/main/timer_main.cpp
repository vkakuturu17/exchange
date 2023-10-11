#include <iostream>
#include <string>
#include <thread>
#include "order_matching_engine/machine_time.h"

using std::cout;
using std::endl;
using std::string;
using Exchange::MachineTimer;

// using std::literals::chrono_literals::operator""ms;
// using std::literals::chrono_literals::operator""s;

int main() {
  MachineTimer timer;
  uint64_t time = timer.get_time();
  std::cout << time << "ns" << std::endl;
  std::cout << time / BILLION << "s" << std::endl;

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  time = timer.get_time();
  std::cout << time << "ns" << std::endl;
  std::cout << time / BILLION << "s" << std::endl;
  return 0;
}