#include "order_matching_engine/order_matching_engine.h"
#include <iostream>
#include <signal.h>

using std::thread;
using std::vector;
using Exchange::OrderMatchingEngine;
using Exchange::MachineTimer;

// #define DEBUG
void busy_print(int i) {
  while (true) {
    std::cout << "busy" << i << std::endl;
    std::this_thread::yield();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

void sigint_handler(int sig) {
  std::cout << "SIGINT received" << std::endl;
  exit(0);
}

int main() {
  // https://thomastrapp.com/posts/signal-handlers-for-multithreaded-c++/
  {
    // Register signal and signal handler
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sigint_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
  }
  // TODO: Make sure sigint is caught by ALL threads

  MachineTimer timer;
  OrderMatchingEngine ome(MachineTimer::get_instance());
  ome.load_orderbook_symbols("../main/symbols.txt");
  std::cout << "Number of symbols: " << ome.get_symbols().size() << std::endl;
    
  vector<thread> threads; // 10 threads
  #ifndef DEBUG
  threads.push_back(thread(&OrderMatchingEngine::add_inbound_packet_to_queue, &ome, std::ref(ome.OUCH_inbound_buffer_queue_)));
  threads.push_back(thread(&OrderMatchingEngine::convert_OUCH_inbound_packet_to_order, &ome, std::ref(ome.OUCH_inbound_buffer_queue_), std::ref(ome.OUCH_inbound_order_queue_)));
  threads.push_back(thread(&OrderMatchingEngine::add_inbound_order_to_orderbook, &ome, std::ref(ome.OUCH_inbound_order_queue_), std::ref(ome.orderbook_symbol_to_execute_queue_)));
  threads.push_back(thread(&OrderMatchingEngine::execute_orderbook_next, &ome, std::ref(ome.orderbook_symbol_to_execute_queue_), std::ref(ome.orderbook_symbol_OUCH_to_output_queue_), std::ref(ome.orderbook_symbol_ITCH_to_output_queue_)));
  threads.push_back(thread(&OrderMatchingEngine::grab_OUCH_outbound_from_orderbook, &ome, std::ref(ome.orderbook_symbol_OUCH_to_output_queue_), std::ref(ome.OUCH_outbound_order_queue_)));
  threads.push_back(thread(&OrderMatchingEngine::grab_ITCH_outbound_from_orderbook, &ome, std::ref(ome.orderbook_symbol_ITCH_to_output_queue_), std::ref(ome.ITCH_outbound_order_queue_)));
  threads.push_back(thread(&OrderMatchingEngine::convert_OUCH_outbound_order_to_packet, &ome, std::ref(ome.OUCH_outbound_order_queue_), std::ref(ome.OUCH_outbound_buffer_queue_)));
  threads.push_back(thread(&OrderMatchingEngine::send_outbound_OUCH_packet, &ome, std::ref(ome.OUCH_outbound_buffer_queue_)));
  threads.push_back(thread(&OrderMatchingEngine::convert_ITCH_outbound_order_to_packet, &ome, std::ref(ome.ITCH_outbound_order_queue_), std::ref(ome.ITCH_outbound_buffer_queue_)));
  threads.push_back(thread(&OrderMatchingEngine::send_outbound_ITCH_packet, &ome, std::ref(ome.ITCH_outbound_buffer_queue_)));
  #endif
  
  #ifdef DEBUG
  threads.push_back(thread(&busy_print, 1));
  threads.push_back(thread(&busy_print, 2));
  #endif

  for (auto& t : threads) {
    t.join();
  }
  
  MachineTimer::destroy_instance();
}