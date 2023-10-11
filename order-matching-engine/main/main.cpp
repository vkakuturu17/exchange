#include <iostream>
#include <memory>
#include <vector>
#include <cstdint>
#include <string.h>
#include <stdio.h>
#include <byteswap.h>
#include "orderbook/orderbook.h"
#include "order_matching_engine/order_matching_engine.h"
#include "struct/nasdaq_ouch.h"

using std::cout;
using std::endl;
using std::unique_ptr;
using std::vector;

using Exchange::OrderBook;
using Exchange::OrderMatchingEngine;
using Exchange::MachineTimer;

int main() {
  // MachineTimer timer;
  // OrderMatchingEngine order_matching_engine(MachineTimer::get_instance());
  // order_matching_engine.load_orderbook_symbols("../tests/files/ome1_orderbooks_sym.txt");
  // std::cout << order_matching_engine.get_symbols().size() << std::endl;

  // unsigned char buffer[32];
  // char c = 'A';
  // uint64_t time = 1234567890;
  // time = bswap_64(time);

  // memcpy(buffer, &c, 1);
  // memcpy(buffer + 1, &time, 8);
  // buffer[31] = '\0';

  // for (int i = 0; i < 32; i++) {
  //   printf("%x\n", buffer[i]);
  // }

  // char c2;
  // uint64_t time2;
  // memcpy(&c2, buffer, 1);
  // memcpy(&time2, buffer + 1, 8);
  // time2 = bswap_64(time2);
  // printf("%c\n", c2);
  // printf("%ld\n", time2);


  // OrderEntry order_entry1("0001", 'B', 1000, "AAPL", 100, -1, "Lehman", 'Y');
  // // InboundOrder* porder_entry1 = new OrderEntry("0001", 'B', 1000, "AAPL", 100, -1, "Lehman", 'Y');
  // // MachineTimer timer;
  // // OrderBook orderbook("AAPL", timer.get_instance());
  // // InboundOrder order1(order_entry1);
  // InboundOrderUniquePtr order_ptr = std::make_unique<InboundOrder>(order_entry1);
  // std::cout << order_ptr->otype << std::endl;
  // std::cout << order_ptr->symbol << std::endl;

  // // InboundOrder* order2 = (InboundOrder*)porder_entry1;
  // // OrderEntry* orderp = (OrderEntry*)(order2);

  // OrderEntry* orderp = dynamic_cast<OrderEntry*>(order_ptr.get());
  // // OrderEntry* orderp = (OrderEntry*)(order_ptr.get());
  // std::cout << orderp->type << std::endl;


  // unsigned char buffer[] = {0x41, 0x00, 0x00, 0x00, 0x10};
  // uint32_t p = 16;
  // memcpy(buffer + 1, &p, 4);
  // char c;
  // uint32_t price;

  // memcpy(&c, buffer, 1);
  // memcpy(&price, buffer + 1, 4);
  // std::cout << c << std::endl;
  // std::cout << price << std::endl;


  // MachineTimer timer;
  // OrderBook orderbook("AAPL", timer.get_instance());
  // OrderEntry order_entry1("0001", 'B', 1000, "AAPL", 100, -1, "Lehman", 'Y');

  // orderbook.add_order(order_entry1);
  // orderbook.process_next_order();

  // std::cout << orderbook.ostringstream_top_of_book().str();
  // std::cout << orderbook.ostringstream_order_ids().str();

  return 0;
}