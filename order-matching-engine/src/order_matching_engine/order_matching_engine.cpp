#include <fstream>
#include <iostream>
#include "parser/parser.h"
#include "order_matching_engine.h"

#define BUFFER_SIZE 128

using std::string;
using std::ifstream;
using Exchange::OUCH_Parser;

namespace Exchange {
  OrderMatchingEngine::OrderMatchingEngine(Timer* timer) : timer_(timer), 
    OUCH_inbound_buffer_queue_(QUEUE_MAX_SIZE), OUCH_outbound_buffer_queue_(QUEUE_MAX_SIZE), ITCH_outbound_buffer_queue_(QUEUE_MAX_SIZE), 
    OUCH_inbound_order_queue_(QUEUE_MAX_SIZE), OUCH_outbound_order_queue_(QUEUE_MAX_SIZE), ITCH_outbound_order_queue_(QUEUE_MAX_SIZE),
    orderbook_symbol_to_execute_queue_(QUEUE_MAX_SIZE),
    orderbook_symbol_OUCH_to_output_queue_(QUEUE_MAX_SIZE), orderbook_symbol_ITCH_to_output_queue_(QUEUE_MAX_SIZE) {

    // Read in textfile for hashmap
  }

  OrderMatchingEngine::~OrderMatchingEngine() {
    __destroy();
  }

  void OrderMatchingEngine::load_orderbook_symbols(string filepath) {
    // Read in textfile for hashmap
    // https://stackoverflow.com/questions/1535249/c-read-file-into-hash-map
    ifstream file(filepath);

    string symbol;
    while (file >> symbol) {
      OrderBookPtr orderbook = std::make_shared<OrderBook>(symbol, timer_);
      orderbooks_[symbol] = orderbook;
    }
  }

  void OrderMatchingEngine::load_orderbook_symbols(std::vector<std::string> symbols) {
    for (auto& symbol : symbols) {
      OrderBookPtr orderbook = std::make_shared<OrderBook>(symbol, timer_);
      orderbooks_[symbol] = orderbook;
    }
  }

  std::vector<std::string> OrderMatchingEngine::get_symbols() const {
    std::vector<std::string> symbols;
    for (auto& symbol : orderbooks_) {
      symbols.push_back(symbol.first);
    }
    return symbols;
  }

  // FSM Logic
  void OrderMatchingEngine::run() {
    // using std::thread;
    // using std::vector;
    // vector<thread> threads; // 10 threads
    // threads.push_back(thread(&OrderMatchingEngine::add_inbound_packet_to_queue, this, std::ref(OUCH_inbound_buffer_queue_)));
    // threads.push_back(thread(&OrderMatchingEngine::convert_OUCH_inbound_packet_to_order, this, std::ref(OUCH_inbound_buffer_queue_), std::ref(OUCH_inbound_order_queue_)));
    // threads.push_back(thread(&OrderMatchingEngine::add_inbound_order_to_orderbook, this, std::ref(OUCH_inbound_order_queue_), std::ref(orderbook_symbol_to_execute_queue_)));
    // threads.push_back(thread(&OrderMatchingEngine::execute_orderbook_next, this, std::ref(orderbook_symbol_to_execute_queue_), std::ref(orderbook_symbol_OUCH_to_output_queue_), std::ref(orderbook_symbol_ITCH_to_output_queue_)));
    // threads.push_back(thread(&OrderMatchingEngine::grab_OUCH_outbound_from_orderbook, this, std::ref(orderbook_symbol_OUCH_to_output_queue_), std::ref(OUCH_outbound_buffer_queue_)));
    // threads.push_back(thread(&OrderMatchingEngine::grab_ITCH_outbound_from_orderbook, this, std::ref(orderbook_symbol_ITCH_to_output_queue_), std::ref(ITCH_outbound_buffer_queue_)));
    // threads.push_back(thread(&OrderMatchingEngine::convert_OUCH_outbound_order_to_packet, this, std::ref(OUCH_outbound_order_queue_), std::ref(OUCH_outbound_buffer_queue_)));
    // threads.push_back(thread(&OrderMatchingEngine::send_outbound_OUCH_packet, this, std::ref(OUCH_outbound_buffer_queue_)));
    // threads.push_back(thread(&OrderMatchingEngine::convert_ITCH_outbound_order_to_packet, this, std::ref(ITCH_outbound_order_queue_), std::ref(ITCH_outbound_buffer_queue_)));
    // threads.push_back(thread(&OrderMatchingEngine::send_outbound_ITCH_packet, this, std::ref(ITCH_outbound_buffer_queue_)));
  }

  void OrderMatchingEngine::add_inbound_packet_to_queue(SPSC_queue<unsigned char*>& inbound_buffer_queue) {
    // TODO: Add TCP socket connection
    // unsigned char* buffer = new unsigned char[BUFFER_SIZE];
    // memcpy(buffer, "OUCH", 4);
    // inbound_buffer_queue.push(buffer);
    // TODO: Add TCP socket connection
  }

  void OrderMatchingEngine::convert_OUCH_inbound_packet_to_order(SPSC_queue<unsigned char*>& inbound_buffer_queue, SPSC_queue<InboundOrderPtr>& inbound_order_queue) {
    while (true) {
      if (inbound_buffer_queue.empty()) {
        std::this_thread::yield();
        continue;
      }

      convert_OUCH_inbound_packet_to_order_IMPL(inbound_buffer_queue, inbound_order_queue);
    }
  }

  void OrderMatchingEngine::convert_OUCH_inbound_packet_to_order_IMPL(SPSC_queue<unsigned char*>& inbound_buffer_queue, SPSC_queue<InboundOrderPtr>& inbound_order_queue) {
    unsigned char* inbound_packet = *inbound_buffer_queue.front();
    InboundOrderPtr order_ptr = OUCH_Parser::parse_inbound(inbound_packet);
    delete[] inbound_packet;
    inbound_order_queue.push(order_ptr);
    inbound_buffer_queue.pop();
  }

  void OrderMatchingEngine::add_inbound_order_to_orderbook(SPSC_queue<InboundOrderPtr>& inbound_order_queue, SPSC_queue<std::string>& orderbook_symbol_to_execute_queue) {
    while (true) {
      if (inbound_order_queue.empty()) {
        std::this_thread::yield();
        continue;
      }

      add_inbound_order_to_orderbook_IMPL(inbound_order_queue, orderbook_symbol_to_execute_queue);
    }
  }

  void OrderMatchingEngine::add_inbound_order_to_orderbook_IMPL(SPSC_queue<InboundOrderPtr>& inbound_order_queue, SPSC_queue<std::string>& orderbook_symbol_to_execute_queue) {
    InboundOrderPtr order = *inbound_order_queue.front();
    { // TODO: Maybe pass along the OrderBook Shared Ptr instead of the symbol
      std::lock_guard lock(orderbooks_[order->symbol]->mutex_lock_); // Lock
      orderbooks_[order->symbol]->add_order(order);
      orderbook_symbol_to_execute_queue.push(order->symbol);
      inbound_order_queue.pop();
    }
  }

  void OrderMatchingEngine::execute_orderbook_next(SPSC_queue<std::string>& orderbook_symbol_to_execute_queue, \
                                                   SPSC_queue<string>& orderbook_symbol_OUCH_to_output_queue, \
                                                   SPSC_queue<string>& orderbook_symbol_ITCH_to_output_queue) {
    while (true) {
      if (orderbook_symbol_to_execute_queue.empty()) {
        std::this_thread::yield();
        continue;
      }

      execute_orderbook_next_IMPL(orderbook_symbol_to_execute_queue, orderbook_symbol_OUCH_to_output_queue, orderbook_symbol_ITCH_to_output_queue);
    }
  }

  void OrderMatchingEngine::execute_orderbook_next_IMPL(SPSC_queue<std::string>& orderbook_symbol_to_execute_queue, \
                                                   SPSC_queue<string>& orderbook_symbol_OUCH_to_output_queue, \
                                                   SPSC_queue<string>& orderbook_symbol_ITCH_to_output_queue) {
    std::string& symbol = *orderbook_symbol_to_execute_queue.front();
    {
      std::lock_guard lock(orderbooks_[symbol]->mutex_lock_); // Lock
      orderbooks_[symbol]->process_next_order();
      orderbook_symbol_OUCH_to_output_queue.push(symbol); // It will pull at most this many orders
      orderbook_symbol_ITCH_to_output_queue.push(symbol); // TODO: Maybe this is not a good way to do this (give a bounds on number of orders to pull from queues)?
                                                          // TODO: Maybe have a mutex locks on the orderbook's queues? Race condition? <- this is done on orderbook side
      orderbook_symbol_to_execute_queue.pop();
    }
  }

  void OrderMatchingEngine::grab_OUCH_outbound_from_orderbook(SPSC_queue<string>& orderbook_symbol_OUCH_to_output_queue, \
                                                              SPSC_queue<OutboundOrderPtr>& outbound_order_queue) {
    while (true) {
      if (orderbook_symbol_OUCH_to_output_queue.empty()) {
        std::this_thread::yield();
        continue;
      }

      grab_OUCH_outbound_from_orderbook_IMPL(orderbook_symbol_OUCH_to_output_queue, outbound_order_queue);
    }
  }

  void OrderMatchingEngine::grab_OUCH_outbound_from_orderbook_IMPL(SPSC_queue<string>& orderbook_symbol_OUCH_to_output_queue, \
                                                                   SPSC_queue<OutboundOrderPtr>& outbound_order_queue) {
    std::string& symbol = *orderbook_symbol_OUCH_to_output_queue.front();
    OrderBookPtr orderbook = orderbooks_[symbol];
    {
      std::lock_guard lock(orderbook->mutex_lock_); // Lock
      uint32_t num_orders = orderbook->get_num_OUCH_outbound_queue();
      for (uint32_t i = 0; i < num_orders; i++) {
        OutboundOrder order = *orderbook->get_OUCH_outbound_order();
        outbound_order_queue.push(std::make_shared<OutboundOrder>(order));
      }
      orderbook_symbol_OUCH_to_output_queue.pop();
    }
  }

  void OrderMatchingEngine::grab_ITCH_outbound_from_orderbook(SPSC_queue<string>& orderbook_symbol_ITCH_to_output_queue, \
                                                              SPSC_queue<ITCH_MessagePtr>& outbound_order_queue) {
    while (true) {
      if (orderbook_symbol_ITCH_to_output_queue.empty()) {
        std::this_thread::yield();
        continue;
      }

      grab_ITCH_outbound_from_orderbook_IMPL(orderbook_symbol_ITCH_to_output_queue, outbound_order_queue);
    }
  }

  void OrderMatchingEngine::grab_ITCH_outbound_from_orderbook_IMPL(SPSC_queue<string>& orderbook_symbol_ITCH_to_output_queue, \
                                                              SPSC_queue<ITCH_MessagePtr>& outbound_order_queue) {
    std::string& symbol = *orderbook_symbol_ITCH_to_output_queue.front();
    OrderBookPtr orderbook = orderbooks_[symbol];
    {
      std::lock_guard lock(orderbook->mutex_lock_); // Lock
      uint32_t num_orders = orderbook->get_num_ITCH_outbound_queue();
      for (uint32_t i = 0; i < num_orders; i++) {
        ITCH_Message order = *orderbook->get_ITCH_outbound_order();
        outbound_order_queue.push(std::make_shared<ITCH_Message>(order));
      }
      orderbook_symbol_ITCH_to_output_queue.pop();
    }
  }

  void OrderMatchingEngine::convert_OUCH_outbound_order_to_packet(SPSC_queue<OutboundOrderPtr>& outbound_order_queue, \
                                                                  SPSC_queue<unsigned char*>& outbound_buffer_queue) {
    while (true) {
      if (outbound_order_queue.empty()) {
        std::this_thread::yield();
        continue;
      }

      convert_OUCH_outbound_order_to_packet_IMPL(outbound_order_queue, outbound_buffer_queue);
    }
  }

  void OrderMatchingEngine::convert_OUCH_outbound_order_to_packet_IMPL(SPSC_queue<OutboundOrderPtr>& outbound_order_queue, \
                                                                  SPSC_queue<unsigned char*>& outbound_buffer_queue) {
    OutboundOrderPtr order_ptr = *outbound_order_queue.front();

    unsigned char* buffer = new unsigned char[BUFFER_SIZE];
    OUCH_Parser::parse_outbound(order_ptr, buffer);

    outbound_buffer_queue.push(buffer);
    outbound_order_queue.pop();
  }

  void OrderMatchingEngine::send_outbound_OUCH_packet(SPSC_queue<unsigned char*>& outbound_buffer_queue) {
    while (true) {
      if (outbound_buffer_queue.empty()) {
        std::this_thread::yield();
        continue;
      }

      send_outbound_OUCH_packet_IMPL(outbound_buffer_queue);
    }
  }

  void OrderMatchingEngine::send_outbound_OUCH_packet_IMPL(SPSC_queue<unsigned char*>& outbound_buffer_queue) {

    unsigned char* buffer = *outbound_buffer_queue.front();

    // TODO: Send TCP packet
    // std::cout << "Sending outbound packet" << std::endl;
    // TODO: Send TCP packet
    delete[] buffer;
  }

  void OrderMatchingEngine::convert_ITCH_outbound_order_to_packet(SPSC_queue<ITCH_MessagePtr>& ITCH_outbound_order_queue, \
                                                                  SPSC_queue<unsigned char*>& ITCH_outbound_buffer_queue) {
    while (true) {
      if (ITCH_outbound_order_queue.empty()) {
        std::this_thread::yield();
        continue;
      }

      convert_ITCH_outbound_order_to_packet_IMPL(ITCH_outbound_order_queue, ITCH_outbound_buffer_queue);
    }
  }

  void OrderMatchingEngine::convert_ITCH_outbound_order_to_packet_IMPL(SPSC_queue<ITCH_MessagePtr>& ITCH_outbound_order_queue, \
                                                                  SPSC_queue<unsigned char*>& ITCH_outbound_buffer_queue) {
    ITCH_MessagePtr message_ptr = *ITCH_outbound_order_queue.front();

    unsigned char* buffer = new unsigned char[BUFFER_SIZE];
    ITCH_Parser::parse_ITCH_outbound(message_ptr, buffer);

    ITCH_outbound_buffer_queue.push(buffer);
    ITCH_outbound_order_queue.pop();
  }

  void OrderMatchingEngine::send_outbound_ITCH_packet(SPSC_queue<unsigned char*>& ITCH_outbound_buffer_queue) {
    while (true) {
      if (ITCH_outbound_buffer_queue.empty()) {
        std::this_thread::yield();
        continue;
      }

      send_outbound_ITCH_packet_IMPL(ITCH_outbound_buffer_queue);
    }
  }

  void OrderMatchingEngine::send_outbound_ITCH_packet_IMPL(SPSC_queue<unsigned char*>& ITCH_outbound_buffer_queue) {
    unsigned char* buffer = *ITCH_outbound_buffer_queue.front();

    // TODO: Send TCP packet
    // std::cout << std::string(buffer) << std::endl;
    // TODO: Send TCP packet

    delete[] buffer;
  }

  void OrderMatchingEngine::__destroy() {
    size_t queue_size = OUCH_inbound_buffer_queue_.size();
    for (size_t i = 0; i < queue_size; ++i) {
      unsigned char* buffer = *OUCH_inbound_buffer_queue_.front();
      delete[] buffer;
      OUCH_inbound_buffer_queue_.pop();
    }
    
    queue_size = OUCH_outbound_buffer_queue_.size();
    for (size_t i = 0; i < queue_size; ++i) {
      unsigned char* buffer = *OUCH_outbound_buffer_queue_.front();
      delete[] buffer;
      OUCH_outbound_buffer_queue_.pop();
    }

    queue_size = ITCH_outbound_buffer_queue_.size();
    for (size_t i = 0; i < queue_size; ++i) {
      unsigned char* buffer = *ITCH_outbound_buffer_queue_.front();
      delete[] buffer;
      ITCH_outbound_buffer_queue_.pop();
    }
  }

} // namespace Exchange

// Order* OrderMatchingEngine::getOrder(string key) {
//   if (orderID_to_order_.contains(key)) return orderID_to_order_[key];
//   else return NULL;
// }