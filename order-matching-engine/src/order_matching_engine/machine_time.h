#pragma once
#include <chrono>

// TODO: Put this inside the namespace. Redefinition error.
#define BILLION 1000000000L // For nano -> second
#define MILLION 1000000L    // For nano -> millisecond
#define THOUSAND 1000       // For nano -> microseconds

namespace Exchange {

class Timer {
  public:
    Timer(){}
    virtual ~Timer(){};

    virtual uint64_t get_time() = 0;
    virtual void reset() = 0;
    virtual uint64_t get_diff() = 0;
    static Timer* get_instance();
    static void destroy_instance();

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
};


class MachineTimer : Timer {
  public:
    MachineTimer();

    uint64_t get_time();
    void reset();
    uint64_t get_diff();

    MachineTimer(const MachineTimer&) = delete;
    void operator=(const MachineTimer&) = delete;

    static Timer* get_instance();
    static void destroy_instance();

  private:
    static MachineTimer* instance_;
    std::chrono::high_resolution_clock::time_point start_time_;
    std::chrono::high_resolution_clock::time_point last_time_;
};


class SimpleMachineTimer : Timer {
  public:
    SimpleMachineTimer();

    uint64_t get_time();
    void reset();
    uint64_t get_diff();

    SimpleMachineTimer(const SimpleMachineTimer&) = delete;
    void operator=(const SimpleMachineTimer&) = delete;

    static Timer* get_instance();
    static void destroy_instance();

  private:
    static SimpleMachineTimer* instance_;
    uint64_t start_time_;
};
} // namespace Exchange