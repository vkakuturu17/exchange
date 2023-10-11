#include "orderbook.h"

namespace Exchange {

std::ostringstream OrderBook::ostringstream_top_of_book() {
  // TODO: Mark argument as const
  uint32_t max_bid_price = get_max_bid_price();
  uint32_t min_ask_price = get_min_ask_price();
  PriceLevelPtr max_bid_priceLevel = price_to_priceLevel_[max_bid_price];
  PriceLevelPtr min_ask_priceLevel = price_to_priceLevel_[min_ask_price];

  std::ostringstream os;

  os << "Top of Book: " << std::endl;
  os << "Ask: ";
  if (min_ask_priceLevel != nullptr) {
    os << "$" << min_ask_price << " | " 
              << min_ask_priceLevel->asks.total_orders << " | "
              << min_ask_priceLevel->asks.total_lots << " | "
              << std::endl;
  } else {
    os << "None" << std::endl;
  }

  os << "Bid: ";
  if (max_bid_priceLevel != nullptr) {
    os << "$" << max_bid_price << " | " 
              << max_bid_priceLevel->bids.total_orders << " | "
              << max_bid_priceLevel->bids.total_lots << " | "
              << std::endl;
  } else {
    os << "None" << std::endl;
  }
  
  return os;
}

std::ostringstream OrderBook::ostringstream_order_ids() {
  std::ostringstream os;
  os << "Order IDs: " << std::endl;
  if (orderID_to_order_.size() == 0) {
    os << "None" << std::endl;
  } else {
    for (auto& order : orderID_to_order_) {
      Order& order_ref = *order.second;
      // std::string order_ID = std::string(order.first + '\0');
      os << order.first << ": " << order_ref.type 
                        << " " << order_ref.quantity 
                        << " @ $" << order_ref.price 
                        << ", Time: " << order_ref.timestamp << "ns" // millisecond
                        << std::endl; 
    }
  }
  return os;
}

}