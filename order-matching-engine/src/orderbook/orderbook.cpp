#include "orderbook.h"
#include <iostream>

namespace Exchange {

  OrderBook::OrderBook(const std::string& stock_symbol, Timer* machine_timer) : stock_symbol_(stock_symbol), machine_timer_(machine_timer)
    // inbound_orders_(256), outbound_orders_(256), outbound_ITCH_messages_(256)
     {

  }

  bool OrderBook::process_next_order() {
    if (inbound_orders_.empty()) return false;
    InboundOrderUniquePtr inbound_order = std::move(inbound_orders_.front()); // End-Of-Life for unique ptr
    InboundOrder* pInboundOrder = inbound_order.get();

    uint64_t timestamp = machine_timer_->get_time();
    switch (pInboundOrder->otype) {
      case 'O':
        // Jump to case label error, caused by scope initiliazation of variables
        // https://stackoverflow.com/questions/5685471/error-jump-to-case-label-in-switch-statement
        {
          OrderEntry* order = dynamic_cast<OrderEntry*>(pInboundOrder);
          if (order) {
            // timestamp = machine_timer_->get_time(); // Removed this to make it fairer for different threads for different order types
            _order_entry(*order, timestamp);
          } else {
            // TODO: Log error
            std::cout << "Error with Order Entry, line: " << __LINE__ << std::endl;
          }
        }
        break;
      case 'X':
        {
          // TODO: Implement
          OrderCancel* order = dynamic_cast<OrderCancel*>(pInboundOrder);
          if (order) {
            // timestamp = machine_timer_->get_time(); // Removed this to make it fairer for different threads for different order types
            _order_cancel(*order, timestamp);
          } else {
            // TODO: Log error
          }
        }
        break;
      case 'U':
        {
          // TODO: Implement
          std::cout << "Order Replace" << std::endl;
        }
        break;
      case 'M':
        {
          // TODO: Implement
          std::cout << "Order Modify" << std::endl;
        }
        break;
      default:
        // TODO: Add error to logging (wrong order type)
        return false;
    }
    inbound_orders_.pop();

    return true;
  }

  void OrderBook::add_order(InboundOrderPtr order) {
    // Create unique ptr
    InboundOrderUniquePtr order_ptr = std::make_unique<InboundOrder>(*order);
    _add_to_inbound_queue(std::move(order_ptr));
  }

  void OrderBook::add_order(OrderEntryPtr order_entry) {
    // Create unique ptr
    OrderEntryUniquePtr order_entry_ptr = std::make_unique<OrderEntry>(*order_entry);
    InboundOrderUniquePtr inorder_entry_ptr = std::move(order_entry_ptr);
    OrderEntry* test = dynamic_cast<OrderEntry*>(inorder_entry_ptr.get());
    _add_to_inbound_queue(std::move(inorder_entry_ptr));
  }

  void OrderBook::add_order(OrderCancelPtr order_cancel) {
    // Create unique ptr
    OrderCancelUniquePtr order_cancel_ptr = std::make_unique<OrderCancel>(*order_cancel);
    InboundOrderUniquePtr inorder_cancel_ptr = std::move(order_cancel_ptr);
    _add_to_inbound_queue(std::move(inorder_cancel_ptr));
  }

  std::vector<std::string> OrderBook::get_order_ids() const {
    std::vector<std::string> order_ids;
    for (auto& order : orderID_to_order_) {
      order_ids.push_back(order.first);
    }
    return order_ids;
  }

  void OrderBook::_order_entry(OrderEntry& new_order, uint64_t timestamp) {
    // Acknowledge receipt of OrderEntry
    OrderAcceptedOutboundUniquePtr ouch_order_accepted_outbound_ptr = _create_order_accepted_outbound(new_order, timestamp, 'L', 'S'); // TODO: Do not hardcode L and S (see struct)
    _add_to_outbound_queue(std::move(ouch_order_accepted_outbound_ptr)); // Outbound
    // TODO: Add order reference number
    ITCH_AddOrderMPIDUniquePtr itch_add_order_outbound_ptr = _create_itch_add_order_mpid(new_order, timestamp, StringHash("NULL"));
    _add_to_itch_queue(std::move(itch_add_order_outbound_ptr)); // Outbound

    switch (new_order.buy_sell_indicator) {
      case 'B':
        if (best_offer_ != nullptr && new_order.price >= get_min_ask_price()) {
          // Crosses spread
          NodePtr& best_offer = best_offer_;
          while (best_offer != nullptr && new_order.shares > 0 && new_order.price >= get_min_ask_price()) {
            BidAsk& asks = best_offer->price_level->asks;
            DoublyLinkedList<OrderPtr>& asks_list = asks.list;
            if (asks_list.empty()) {
              // No sellers at this price level, continue to next price level
              best_offer = best_offer->next;
              continue;
            }
            
            OrderPtr ask_order = asks_list.front();
            if (ask_order->quantity > new_order.shares) {
              // Resting order is larger than new_order
              ask_order->quantity -= new_order.shares;
              asks.total_lots -= new_order.shares; // TODO: Refactor this
              
              // Send outbound message to both parties
              // TODO: Refactor outbound messages
              uint32_t match_number = increment_match_number();
              uint64_t timestamp = machine_timer_->get_time();
              OrderExecutedOutboundUniquePtr passive_outbound_msg_ptr = 
                _create_order_executed_outbound(ask_order->mpid, ask_order->order_token, timestamp, new_order.shares, ask_order->price, match_number);
              _add_to_outbound_queue(std::move(passive_outbound_msg_ptr)); // Outbound
              OrderExecutedOutboundUniquePtr agressive_outbound_msg_ptr = 
                _create_order_executed_outbound(std::string(new_order.mpid, MPID_LEN), std::string(new_order.order_token, ORDER_TOKEN_LEN), \
                                                timestamp, new_order.shares, ask_order->price, match_number);
              _add_to_outbound_queue(std::move(agressive_outbound_msg_ptr)); // Outbound
              // TODO: Add order reference number
              ITCH_OrderExecutedPriceUniquePtr itch_order_executed_ptr = _create_itch_order_executed_price(timestamp, StringHash("NULL"), new_order.shares, match_number, 'Y', ask_order->price);
              _add_to_itch_queue(std::move(itch_order_executed_ptr)); // Outbound

              new_order.shares = 0;
            } else {
              // Resting order is smaller than new_order
              new_order.shares -= ask_order->quantity;
              asks_list.pop_front();
              asks.total_lots -= ask_order->quantity; // TODO: Refactor this
              asks.total_orders -= 1;

              NodePtr temp = best_offer->next; // TODO: Memory leak here?
              best_offer.reset();
              best_offer = temp;

              
              orderID_to_order_.erase(ask_order->mpid + ask_order->order_token);
              if (ask_order->quantity != 0) {
                // TODO: Refactor outbound messages
                uint32_t match_number = increment_match_number();
                uint64_t timestamp = machine_timer_->get_time();
                OrderExecutedOutboundUniquePtr passive_outbound_msg_ptr = _create_order_executed_outbound(ask_order->mpid, ask_order->order_token, timestamp, ask_order->quantity, ask_order->price, match_number); // TODO: Add correct timestamp
                _add_to_outbound_queue(std::move(passive_outbound_msg_ptr)); // Outbound
                OrderExecutedOutboundUniquePtr agressive_outbound_msg_ptr = _create_order_executed_outbound(std::string(new_order.mpid, MPID_LEN), std::string(new_order.order_token, ORDER_TOKEN_LEN), timestamp, ask_order->quantity, ask_order->price, match_number); // TODO: Add correct timestamp
                _add_to_outbound_queue(std::move(agressive_outbound_msg_ptr)); // Outbound
                // TODO: Add order reference number
                ITCH_OrderExecutedPriceUniquePtr itch_order_executed_ptr = _create_itch_order_executed_price(timestamp, StringHash("NULL"), ask_order->quantity, match_number, 'Y', ask_order->price);
                _add_to_itch_queue(std::move(itch_order_executed_ptr)); // Outbound
              }
            }
          }
        }

        // Rest any leftover of the order
        if (best_offer_ == nullptr || new_order.shares > 0 || new_order.price < get_min_ask_price()) {
          OrderPtr order = _create_internal_order(new_order, timestamp);
          _insert_order_into_orderbook(order, new_order.buy_sell_indicator);
        }
        break;
      case 'S':
        // Sell
        if (best_bid_ != nullptr && new_order.price <= get_max_bid_price()) {
          // Crosses spread
          NodePtr& best_bid = best_bid_;
          while (best_bid != nullptr && new_order.shares > 0 && new_order.price <= get_max_bid_price()) {
            BidAsk& bids = best_bid->price_level->bids;
            DoublyLinkedList<OrderPtr>& bids_list = bids.list;
            if (bids_list.empty()) {
              // No sellers at this price level, continue to next price level
              best_bid = best_bid->next;
              continue;
            }
            
            OrderPtr bid_order = bids_list.front();
            if (bid_order->quantity > new_order.shares) {
              // Order is larger than new_order
              bid_order->quantity -= new_order.shares;
              bids.total_lots -= new_order.shares; // TODO: Refactor this

              // TODO: Refactor outbound messages
              uint32_t match_number = increment_match_number();
              uint64_t timestamp = machine_timer_->get_time();
              OrderExecutedOutboundUniquePtr passive_outbound_msg_ptr = _create_order_executed_outbound(bid_order->mpid, bid_order->order_token, timestamp, new_order.shares, bid_order->price, match_number); // TODO: Add correct timestamp
              _add_to_outbound_queue(std::move(passive_outbound_msg_ptr)); // Outbound
              OrderExecutedOutboundUniquePtr agressive_outbound_msg_ptr = _create_order_executed_outbound(std::string(new_order.mpid, MPID_LEN), std::string(new_order.order_token, ORDER_TOKEN_LEN), timestamp, new_order.shares, bid_order->price, match_number); // TODO: Add correct timestamp
              _add_to_outbound_queue(std::move(agressive_outbound_msg_ptr)); // Outbound
              // TOOD: Add order reference number
              ITCH_OrderExecutedPriceUniquePtr itch_order_executed_ptr = _create_itch_order_executed_price(timestamp, StringHash("NULL"), new_order.shares, match_number, 'Y', bid_order->price);
              _add_to_itch_queue(std::move(itch_order_executed_ptr)); // Outbound

              new_order.shares = 0;
            } else {
              // Order is smaller than new_order
              new_order.shares -= bid_order->quantity;
              bids_list.pop_front();
              bids.total_lots -= bid_order->quantity; // TODO: Refactor this
              bids.total_orders -= 1;

              // NodePtr temp = best_bid->next; // TODO: Memory leak here?
              // best_bid.reset();
              // best_bid = temp;
              if (bids_list.empty()) best_bid = best_bid->next;

              orderID_to_order_.erase(bid_order->mpid + bid_order->order_token);
              
              if (bid_order->quantity != 0) {
                // TODO: Refactor outbound messages
                uint32_t match_number = increment_match_number();
                uint64_t timestamp = machine_timer_->get_time();
                OrderExecutedOutboundUniquePtr passive_outbound_msg_ptr = _create_order_executed_outbound(bid_order->mpid, bid_order->order_token, timestamp, bid_order->quantity, bid_order->price, match_number); // TODO: Add correct timestamp
                _add_to_outbound_queue(std::move(passive_outbound_msg_ptr)); // Outbound
                OrderExecutedOutboundUniquePtr agressive_outbound_msg_ptr = _create_order_executed_outbound(std::string(new_order.mpid, MPID_LEN), std::string(new_order.order_token, ORDER_TOKEN_LEN), timestamp, bid_order->quantity, bid_order->price, match_number); // TODO: Add correct timestamp
                _add_to_outbound_queue(std::move(agressive_outbound_msg_ptr)); // Outbound
                // TODO: Add order reference number
                ITCH_OrderExecutedPriceUniquePtr itch_order_executed_ptr = _create_itch_order_executed_price(timestamp, StringHash("NULL"), new_order.shares, match_number, 'Y', bid_order->price);
                _add_to_itch_queue(std::move(itch_order_executed_ptr)); // Outbound
              }
            }
          }
        }

        // Rest any leftover of the order
        if (best_bid_ == nullptr || new_order.shares > 0 || new_order.price > get_max_bid_price()) {
          OrderPtr order = _create_internal_order(new_order, timestamp);
          _insert_order_into_orderbook(order, new_order.buy_sell_indicator);
        }
        break;
      case 'T':
        // Sell Short
        break;
      case 'E':
        // Sell Short Exempt
        break;
      default:
        break;
      
    }
    
  }

  void OrderBook::_order_cancel(const OrderCancel& order_cancel, uint64_t timestamp) {
    if (orderID_to_order_.find(AddCharArray(order_cancel.mpid, MPID_LEN, order_cancel.order_token, ORDER_TOKEN_LEN)) == orderID_to_order_.end()) {
      // Order does not exist
      OutboundOrderUniquePtr outbound_msg_ptr = _create_order_cancel_rejected_outbound(order_cancel, timestamp);
      _add_to_outbound_queue(std::move(outbound_msg_ptr));
      // Don't need to send ITCH message
      // TODO: Add log here and remove below print statement
      std::cout << "Order does not exist" << std::endl;
      return ;
    }
    OrderCanceledOutboundUniquePtr outbound_msg_ptr = _create_order_canceled_outbound(order_cancel, timestamp);
    outbound_msg_ptr->reason = 'U'; // User requested cancel

    OrderPtr order = orderID_to_order_[AddCharArray(order_cancel.mpid, MPID_LEN, order_cancel.order_token, ORDER_TOKEN_LEN)];
    if (order->quantity > order_cancel.shares) {
      outbound_msg_ptr->decrement_shares = order_cancel.shares;
      order->quantity -= order_cancel.shares;
    } else {
      outbound_msg_ptr->decrement_shares = order->quantity;
      order->quantity = 0;
    }

    // Decrease lot in price level, remove order, or remove price level if no more lots
    if (order->type == 'B') {
      uint32_t price = order->price;
      BidAsk& bids = price_to_priceLevel_[price]->bids;
      bids.total_lots -= outbound_msg_ptr->decrement_shares;
      if (order->quantity == 0) bids.total_orders -= 1;
      /**
       * TODO: This will create hanging PLs and Orders in the data structure with size 0. 
       * TODO: The idea is that they will be executed on but not send out a message. 
       * TODO: This is not the best way to do it.
       * 
       * TODO: Theoretically, I need to get rid of it in all data structures (order map, price level map, best bid/ask LL node)
       */
    }

    _add_to_outbound_queue(std::move(outbound_msg_ptr));
    ITCH_OrderCancelUniquePtr itch_msg_ptr = _create_itch_order_cancel(timestamp, StringHash(order->mpid + order->order_token), order_cancel.shares);
    _add_to_itch_queue(std::move(itch_msg_ptr));
  }



  OrderAcceptedOutboundUniquePtr OrderBook::_create_order_accepted_outbound(const OrderEntry& new_order, \
    uint64_t timestamp, char order_state, char bbo_weight_indicator) {
    OrderAcceptedOutboundUniquePtr msg = std::make_unique<OrderAcceptedOutbound>(timestamp, std::string(new_order.order_token, ORDER_TOKEN_LEN), \
      new_order.buy_sell_indicator, new_order.shares, new_order.stock_sym, new_order.price, new_order.time_in_force, \
      std::string(new_order.mpid, MPID_LEN), new_order.display, AddCharArray(new_order.mpid, MPID_LEN, new_order.order_token, ORDER_TOKEN_LEN), \
      order_state, bbo_weight_indicator); // TODO: Change OrderID input
    return msg;
  }

  OrderCanceledOutboundUniquePtr OrderBook::_create_order_canceled_outbound(const OrderCancel& order_cancel, uint64_t timestamp) {
    OrderCanceledOutboundUniquePtr msg = std::make_unique<OrderCancelledOutbound>(timestamp, \
    std::string(order_cancel.mpid, MPID_LEN), std::string(order_cancel.order_token, ORDER_TOKEN_LEN), \
      0, '\0');
    return msg;
  }

  CancelRejectOutboundUniquePtr OrderBook::_create_order_cancel_rejected_outbound(const OrderCancel& order_cancel, \
    uint64_t timestamp) {
    CancelRejectOutboundUniquePtr msg = std::make_unique<CancelRejectOutbound>(timestamp, std::string(order_cancel.mpid, MPID_LEN), \
      std::string(order_cancel.order_token, ORDER_TOKEN_LEN));
    return msg;
  }

  OrderExecutedOutboundUniquePtr OrderBook::_create_order_executed_outbound(str_cref mpid, str_cref order_token, uint64_t timestamp, uint32_t shares, uint32_t price, uint32_t match_number) {
    OrderExecutedOutboundUniquePtr msg = std::make_unique<OrderExecutedOutbound>(timestamp, mpid, \
      order_token, shares, price, '_', match_number);
    return msg;
  }

  OrderPtr OrderBook::_create_internal_order(const OrderEntry& new_order, uint64_t timestamp) {
    OrderPtr order = std::make_shared<Order>(new_order.buy_sell_indicator, \
    StringFromArray(new_order.order_token, ORDER_TOKEN_LEN), StringFromArray(new_order.mpid, MPID_LEN), \
      new_order.price, new_order.shares, timestamp);
    return order;
  }

  void OrderBook::_insert_order_into_orderbook(OrderPtr& order, char buy_sell_indicator) {
    PriceLevelPtr price_level;
    if (!price_to_priceLevel_.contains(order->price)) {
      price_level = std::make_shared<PriceLevel>(order->price, 1, order->quantity, order, buy_sell_indicator);
      price_to_priceLevel_[order->price] = price_level;

      NodePtr node = std::make_shared<Node>();
      node->price_level = price_level;

      switch(buy_sell_indicator) {
        case 'B':
          _insert_best_bid(node);
          break;
        case 'S':
          _insert_best_offer(node);
          break;
        // TODO: Add support for sell short and sell short exempt
        default:
          // TODO: Add logging if wrong value
          break;
      }

    } else {
      price_level = price_to_priceLevel_[order->price];
      price_level->add_order(order, buy_sell_indicator);
    }

    orderID_to_order_[order->mpid + order->order_token] = order;
  }


  void OrderBook::_insert_best_bid(NodePtr& node) {
    if (best_bid_ == nullptr) {
      best_bid_ = node;
      return;
    }

    if (node->price_level->price > best_bid_->price_level->price) {
      node->next = best_bid_;
      best_bid_ = node;
    }

    NodePtr current = best_bid_;
    while (current->next != nullptr && node->price_level < current->next->price_level) {
      current = current->next;
    }

    node->next = current->next;
    current->next = node;
  }

  void OrderBook::_insert_best_offer(NodePtr& node) {
    if (best_offer_ == nullptr) {
      best_offer_ = node;
      return;
    }

    if (node->price_level->price < best_offer_->price_level->price) {
      node->next = best_offer_;
      best_offer_ = node;
    }

    NodePtr current = best_offer_;
    while (current->next != nullptr && node->price_level > current->next->price_level) {
      current = current->next;
    }

    node->next = current->next;
    current->next = node;
  }

  ITCH_AddOrderMPIDUniquePtr OrderBook::_create_itch_add_order_mpid(const OrderEntry& new_order, uint64_t timestamp, uint64_t order_ref_num) {
    ITCH_AddOrderMPIDUniquePtr msg = std::make_unique<ITCH_AddOrderMPID>(0, 0, timestamp, order_ref_num, new_order.buy_sell_indicator, \
      new_order.shares, new_order.stock_sym, new_order.price, new_order.mpid); // TODO: change to match struct instead of default 0
    return msg;
  }

  ITCH_OrderExecutedPriceUniquePtr OrderBook::_create_itch_order_executed_price(uint64_t timestamp, uint64_t order_ref_num, uint32_t executed_shares, uint64_t match_num, char printable, uint32_t execution_price) {
    ITCH_OrderExecutedPriceUniquePtr msg = std::make_unique<ITCH_OrderExecutedPrice>(0, 0, timestamp, order_ref_num, executed_shares, \
      match_num, printable, execution_price); // TODO: change to match struct instead of default 0
    return msg;
  }

  ITCH_OrderCancelUniquePtr OrderBook::_create_itch_order_cancel(uint64_t timestamp, uint64_t order_ref_number, uint32_t canceled_shares) {
    ITCH_OrderCancelUniquePtr msg = std::make_unique<ITCH_OrderCancel>(0, 0, timestamp, order_ref_number, canceled_shares); // TODO: change to match struct instead of default 0
    return msg;
  }
} // namespace Exchange