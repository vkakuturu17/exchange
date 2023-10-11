#include <iostream>
#include <memory>
#include <vector>
#include <cstdint>
#include <thread>
#include "orderbook/orderbook.h"
#include "order_matching_engine/order_matching_engine.h"
#include "order_matching_engine/machine_time.h"
#include "struct/nasdaq_ouch.h"

using std::cout;
using std::endl;
using std::unique_ptr;
using std::vector;

using Exchange::OrderBook;
using Exchange::SimpleMachineTimer;
using Exchange::MachineTimer;

int main() {
  MachineTimer timer;
  OrderBook orderbook("AAPL", timer.get_instance());

  OrderEntry order_entry1("0001", 'B', 1000, "AAPL", 100, -1, "Lehman", 'Y');
  OrderEntry order_entry2("0002", 'B', 1000, "AAPL", 99, -1, "Lehman", 'Y');
  OrderEntry order_entry3("0003", 'S', 1000, "AAPL", 101, -1, "Lehman", 'Y');
  OrderEntry order_entry5("0005", 'B', 500, "AAPL", 100, -1, "Lehman", 'Y');
  // OrderEntryUniquePtr order_entry1_ptr = std::make_unique<OrderEntry>(order_entry1);
  orderbook.add_order(std::make_shared<OrderEntry>(order_entry1));
  
  // cout << "Num orders in queue: " << orderbook.get_num_inbound_queue() << endl;
  
  bool added_order = orderbook.process_next_order();
  // std::cout << added_order << std::endl;
  orderbook.add_order(std::make_shared<OrderEntry>(order_entry2));
  orderbook.process_next_order();
  orderbook.add_order(std::make_shared<OrderEntry>(order_entry3));
  orderbook.process_next_order();
  orderbook.add_order(std::make_shared<OrderEntry>(order_entry5));
  orderbook.process_next_order();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // cout << "Best bid price: " << orderbook.get_max_bid_price() << endl;
  // cout << "Best offer price: " << orderbook.get_min_ask_price() << endl;

  std::cout << orderbook.ostringstream_top_of_book().str();
  std::cout << orderbook.ostringstream_order_ids().str();


  OrderEntry order_entry4("0004", 'S', 20, "AAPL", 100, -1, "Lehman", 'Y');
  orderbook.add_order(std::make_shared<OrderEntry>(order_entry4));
  orderbook.process_next_order();

  std::cout << orderbook.ostringstream_top_of_book().str();

  OrderEntry order_entry7("0007", 'S', 1000, "AAPL", 100, -1, "Lehman", 'Y');
  orderbook.add_order(std::make_shared<OrderEntry>(order_entry7));
  orderbook.process_next_order();

  std::cout << orderbook.ostringstream_top_of_book().str();

  OrderEntry order_entry6("0006", 'B', 1500, "AAPL", 101, -1, "Lehman", 'Y');
  orderbook.add_order(std::make_shared<OrderEntry>(order_entry6));
  orderbook.process_next_order();

  std::cout << orderbook.ostringstream_top_of_book().str();
  
  std::cout << orderbook.ostringstream_order_ids().str();

  // Try to cancel order
  OrderCancel order_cancel1("Lehman", "0002", 100);
  orderbook.add_order(std::make_shared<OrderCancel>(order_cancel1));
  orderbook.process_next_order();

  std::cout << orderbook.ostringstream_order_ids().str();

  OrderCancel order_cancel2("Lehman", "0002", 900);
  orderbook.add_order(std::make_shared<OrderCancel>(order_cancel2));
  orderbook.process_next_order();

  std::cout << orderbook.ostringstream_order_ids().str();


  // https://www.bogotobogo.com/cplusplus/upcasting_downcasting.php
  // InboundOrderUniquePtr pOrder;
  // OrderEntryUniquePtr order_entry1 = std::make_unique<OrderEntry>("3409587", 'B', 1000, "AAPL", 100, -1, "Lehman", 'Y');
  // pOrder = std::move(order_entry1);
  // cout << pOrder->otype << endl;

  // InboundOrder* order = pOrder.get();
  // OrderEntry* order_entry = dynamic_cast<OrderEntry*>(order);
  // OrderEntry oe = *order_entry;
  // cout << oe.type << endl;

  return 0;
}