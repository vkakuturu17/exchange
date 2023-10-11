/**
 * @file structs.hpp
 * 
 */
#pragma once
#include <stdint.h>
#include <string>
#include "doublylinkedlist/dll.h"

typedef struct Order {
  char type;
  std::string order_token;
  std::string mpid;
  uint32_t price;
  uint32_t quantity;
  uint64_t timestamp;

  Order() {}
  Order(const char& ty, const std::string& ord, const std::string& mp, uint32_t pr, uint32_t qu, uint64_t ti) 
    : type(ty), order_token(ord), mpid(mp), price(pr), quantity(qu), timestamp(ti) {}

  bool operator<(const Order& other) const {
    return timestamp < other.timestamp;
  }
} Order;
typedef std::shared_ptr<Order> OrderPtr;

typedef struct BidAsk {
  uint32_t total_lots;
  uint32_t total_orders;
  DoublyLinkedList<OrderPtr> list;

  BidAsk() : total_lots(0), total_orders(0) {}
  ~BidAsk() {
    list.clear();
  }
} BidAsk;

class PriceLevel {
  public:
    uint32_t price;
    // uint32_t total_orders;
    // uint32_t total_lots;
    // DoublyLinkedList<OrderPtr> bids;
    BidAsk bids;
    BidAsk asks;

    PriceLevel(uint32_t pr, uint32_t to, uint32_t tl, OrderPtr ord, char buy_sell_indicator) 
      : price(pr) {
        switch(buy_sell_indicator) {
          case 'B':
            bids.list.push_back(ord);
            bids.total_lots += tl;
            bids.total_orders += to;
            break;
          case 'S':
            asks.list.push_back(ord);
            asks.total_lots += tl;
            asks.total_orders += to;
            break;
          default:
            break;
        }
      }

    ~PriceLevel() {
      // Make sure to send order cancel here
    }

    void add_order(OrderPtr ord, char buy_sell_indicator) {
      switch(buy_sell_indicator) {
        case 'B':
          bids.list.push_back(ord);
          bids.total_lots += ord->quantity;
          bids.total_orders += 1;
          break;
        case 'S':
          asks.list.push_back(ord);
          asks.total_lots += ord->quantity;
          asks.total_orders += 1;
          break;
        default:
          break;
      }
    }

    bool operator<(const PriceLevel& other) const {
      return price < other.price;
    }
};
typedef std::shared_ptr<PriceLevel> PriceLevelPtr;
