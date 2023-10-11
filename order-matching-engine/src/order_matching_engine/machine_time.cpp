#include "machine_time.h"

namespace Exchange {
// https://stackoverflow.com/questions/14331469/undefined-reference-to-static-variable
SimpleMachineTimer* SimpleMachineTimer::instance_ = nullptr;
MachineTimer* MachineTimer::instance_ = nullptr;

// MachineTimer //

MachineTimer::MachineTimer() {
  start_time_ = std::chrono::high_resolution_clock::now();
}

uint64_t MachineTimer::get_time() {
  auto end_time = std::chrono::high_resolution_clock::now();
  last_time_ = end_time;
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time_);
  return duration.count();
}

void MachineTimer::reset() {
  start_time_ = std::chrono::high_resolution_clock::now();
}

uint64_t MachineTimer::get_diff() {
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - last_time_);
  return duration.count();
}

Timer* MachineTimer::get_instance() {
  if (instance_ == nullptr) {
    MachineTimer::instance_ = new MachineTimer();
  }
  return instance_;
}

void MachineTimer::destroy_instance() {
  if (instance_ != nullptr) {
    delete instance_;
    instance_ = nullptr;
  }
}

// SimpleMachineTimer //

SimpleMachineTimer::SimpleMachineTimer() {
  start_time_ = 0;
}

uint64_t SimpleMachineTimer::get_time() {
  return ++start_time_;
}

void SimpleMachineTimer::reset() {
  start_time_ = 0;
}

uint64_t SimpleMachineTimer::get_diff() {
  // Will always be 1 because we increment 1 everytime get_time is called.
  return 1;
}

Timer* SimpleMachineTimer::get_instance() {
  if (instance_ == nullptr) {
    SimpleMachineTimer::instance_ = new SimpleMachineTimer();
  }
  return instance_;
}

void SimpleMachineTimer::destroy_instance() {
  if (instance_ != nullptr) {
    delete instance_;
    instance_ = nullptr;
  }
}

} // namespace Exchange
