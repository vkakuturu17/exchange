#include <string.h>
#include <byteswap.h>
#include <iostream>
#include "parser.h"

namespace Exchange {

  // class OUCH_Parser //

  InboundOrderPtr OUCH_Parser::parse_inbound(unsigned char* msg) {
    InboundOrderPtr order_ptr = std::make_shared<InboundOrder>();
    order_ptr->otype = msg[0];
    switch (order_ptr->otype) {
      case 'O':
        {
          OrderEntry order = _parse_order_entry(msg);
          order_ptr = std::make_shared<OrderEntry>(order);
        }
        break;
      case 'X':
        {
          OrderCancel order = _parse_order_cancel(msg);
          order_ptr = std::make_shared<OrderCancel>(order);
        }
        break;
      case 'U':
        {
          // OrderReplace order = _parse_order_replace(msg);
        }
        break;
      case 'M':
        {
          // OrderModify order = _parse_order_modify(msg);
        }
        break;
      default:
        // TODO: Log error
        break;
    }
    return order_ptr;
  }

  void OUCH_Parser::parse_outbound(OutboundOrderPtr order, unsigned char buffer[]) {
    switch (order->otype) {
      case 'S':
        {
          SystemEventMessage* msg = dynamic_cast<SystemEventMessage*>(order.get());
          _parse_system_event_message(*msg, buffer);
        }      
        break;
      case 'A':
        {
          OrderAcceptedOutbound* msg = dynamic_cast<OrderAcceptedOutbound*>(order.get());
          _parse_order_accepted_outbound(*msg, buffer);
        }
        break;
      // case 'U':
      //   _parse_order_replaced_outbound(dynamic_cast<OrderReplacedOutbound&>(order), buffer);
      //   break;
      case 'C':
        {
          OrderCancelledOutbound* msg = dynamic_cast<OrderCancelledOutbound*>(order.get());
          _parse_order_cancelled_outbound(*msg, buffer);
        }
        break;
      case 'E':
        {
          OrderExecutedOutbound* msg = dynamic_cast<OrderExecutedOutbound*>(order.get());
          _parse_order_executed_outbound(*msg, buffer);
        }
        break;
      // case 'J':
      //   _parse_rejected_message_outbound(dynamic_cast<RejectedMessageOutbound&>(order), buffer);
      //   break;
      // case 'I':
      //   _parse_cancel_reject_outbound(dynamic_cast<CancelRejectOutbound&>(order), buffer);
      //   break;
      // case 'T':
      //   _parse_order_priority_update_outbound(dynamic_cast<OrderPriorityUpdateOutbound&>(order), buffer);
      //   break;
      default:
        // TODO: Log error
        std::cout << "Error: Invalid outbound order type" << std::endl;
        break;
    }
  }

  void OUCH_Parser::parse_OUCH_inbound_into_buffer(InboundOrderPtr order, unsigned char buffer[]) {
    switch (order->otype) {
      case 'O':
        {
          OrderEntry* msg = dynamic_cast<OrderEntry*>(order.get());
          _parse_order_entry_into_buffer(*msg, buffer);
        }
        break;
      default:
        // TODO: Log error instead of printing to std::cout
        std::cout << "Error: Invalid inbound order type: " << order->otype << std::endl;
        break;
    }
  }

  OrderEntry OUCH_Parser::_parse_order_entry(unsigned char* msg) {
    OrderEntry order;
    // memcpy(&order.otype, msg, 1);
    memcpy(&order.order_token, msg + 1, 14);
    memcpy(&order.buy_sell_indicator, msg + 15, 1);
    memcpy(&order.shares, msg + 16, 4);
    memcpy(&order.stock_sym, msg + 20, 8);
    memcpy(&order.price, msg + 28, 4);
    memcpy(&order.time_in_force, msg + 32, 4);
    memcpy(&order.mpid, msg + 36, 4);
    memcpy(&order.display, msg + 40, 1); 
    memcpy(&order.capacity, msg + 41, 1);
    memcpy(&order.intermarket_sweep_eligibility, msg + 42, 1);
    memcpy(&order.minimum_quantity, msg + 43, 4);
    memcpy(&order.cross_type, msg + 47, 1);
    memcpy(&order.customer_type, msg + 48, 1);

    return order;
  }

  OrderCancel OUCH_Parser::_parse_order_cancel(unsigned char* msg) {
    OrderCancel order;
    // memcpy(&order.type, msg, 1);
    memcpy(&order.order_token, msg + 1, 14);
    memcpy(&order.shares, msg + 15, 4);
    memcpy(&order.mpid, msg + 19, 4);

    return order;
  }

  void OUCH_Parser::_parse_order_entry_into_buffer(OrderEntry& order, unsigned char buffer[]) {
    memcpy(buffer, &order.type, 1);
    memcpy(buffer + 1, &order.order_token, 14);
    memcpy(buffer + 15, &order.buy_sell_indicator, 1);
    memcpy(buffer + 16, &order.shares, 4);
    memcpy(buffer + 20, &order.stock_sym, 8);
    memcpy(buffer + 28, &order.price, 4);
    memcpy(buffer + 32, &order.time_in_force, 4);
    memcpy(buffer + 36, &order.mpid, 4);
    memcpy(buffer + 40, &order.display, 1);
    memcpy(buffer + 41, &order.capacity, 1);
    memcpy(buffer + 42, &order.intermarket_sweep_eligibility, 1);
    memcpy(buffer + 43, &order.minimum_quantity, 4);
    memcpy(buffer + 47, &order.cross_type, 1);
    memcpy(buffer + 48, &order.customer_type, 1);
  }

  void OUCH_Parser::_parse_system_event_message(SystemEventMessage& msg, unsigned char buffer[]) {
    memcpy(buffer, &msg.type, 1);
    memcpy(buffer + 1, &msg.timestamp, 8);
    memcpy(buffer + 9, &msg.event_code, 1);
  }

  void OUCH_Parser::_parse_order_accepted_outbound(OrderAcceptedOutbound& msg, unsigned char buffer[]) {
    // TODO: May need to byte swap
    memcpy(buffer, &msg.type, 1);
    memcpy(buffer + 1, &msg.timestamp, 8);
    memcpy(buffer + 9, &msg.order_token, 14);
    memcpy(buffer + 23, &msg.buy_sell_indicator, 1);
    memcpy(buffer + 24, &msg.shares, 4);
    memcpy(buffer + 28, &msg.stock_sym, 8);
    memcpy(buffer + 36, &msg.price, 4);
    memcpy(buffer + 40, &msg.time_in_force, 4);
    memcpy(buffer + 44, &msg.mpid, 4);
    memcpy(buffer + 48, &msg.display, 1);
    memcpy(buffer + 49, &msg.orderID, 8);
    memcpy(buffer + 57, &msg.capacity, 1);
    memcpy(buffer + 58, &msg.intermarket_sweep_eligibility, 1);
    memcpy(buffer + 59, &msg.minimum_quantity, 4);
    memcpy(buffer + 63, &msg.cross_type, 1);
    memcpy(buffer + 64, &msg.order_state, 1);
    memcpy(buffer + 65, &msg.bbo_weight_indicator, 1);
  }

  void OUCH_Parser::_parse_order_replaced_outbound(OrderReplacedOutbound& msg, unsigned char buffer[]) {
    // memcpy(buffer, &msg.otype, 1);
    // memcpy(buffer + 1, &msg.timestamp, 8);
    // memcpy(buffer + 9, &msg.order_token, 8);
    // memcpy(buffer + 17, &msg.original_order_token, 8);
    // memcpy(buffer + 25, &msg.shares, 4);
    // memcpy(buffer + 29, &msg.price, 8);
  }

  void OUCH_Parser::_parse_order_cancelled_outbound(OrderCancelledOutbound& msg, unsigned char buffer[]) {
    memcpy(buffer, &msg.type, 1);
    memcpy(buffer + 1, &msg.timestamp, 8);
    memcpy(buffer + 9, &msg.order_token, 14);
    memcpy(buffer + 23, &msg.decrement_shares, 4);
    memcpy(buffer + 27, &msg.reason, 1);
    memcpy(buffer + 28, &msg.mpid, 4);
  }

  void OUCH_Parser::_parse_order_executed_outbound(OrderExecutedOutbound& msg, unsigned char buffer[]) {
    memcpy(buffer, &msg.type, 1);
    memcpy(buffer + 1, &msg.timestamp, 8);
    memcpy(buffer + 9, &msg.order_token, 14);
    memcpy(buffer + 23, &msg.executed_shares, 4);
    memcpy(buffer + 27, &msg.executed_price, 4);
    memcpy(buffer + 31, &msg.liquidity_flag, 1);
    memcpy(buffer + 32, &msg.match_number, 8);
    memcpy(buffer + 40, &msg.mpid, 4);
  }

  void OUCH_Parser::_parse_rejected_message_outbound(RejectedMessageOutbound& msg, unsigned char buffer[]) {
    // memcpy(buffer, &msg.otype, 1);
    // memcpy(buffer + 1, &msg.timestamp, 8);
    // memcpy(buffer + 9, &msg.order_token, 8);
    // memcpy(buffer + 17, &msg.reject_code, 1);
  }

  void OUCH_Parser::_parse_cancel_reject_outbound(CancelRejectOutbound& msg, unsigned char buffer[]) {
    // memcpy(buffer, &msg.otype, 1);
    // memcpy(buffer + 1, &msg.timestamp, 8);
    // memcpy(buffer + 9, &msg.order_token, 8);
    // memcpy(buffer + 17, &msg.reject_code, 1);
  }

  void OUCH_Parser::_parse_order_priority_update_outbound(OrderPriorityUpdateOutbound& msg, unsigned char buffer[]) {
    // memcpy(buffer, &msg.otype, 1);
    // memcpy(buffer + 1, &msg.timestamp, 8);
    // memcpy(buffer + 9, &msg.order_token, 8);
    // memcpy(buffer + 17, &msg.priority, 1);
  }
  
  ///////////////////////
  // class ITCH_Parser //
  ///////////////////////

  void ITCH_Parser::parse_ITCH_outbound(ITCH_MessagePtr msg, unsigned char buffer[]) {
    switch (msg->otype) {
      case 'F':
        {
          ITCH_AddOrderMPID* add_order = dynamic_cast<ITCH_AddOrderMPID*>(msg.get());
          _parse_ITCH_add_order(*add_order, buffer);
        }
        break;
      case 'C':
        {
          ITCH_OrderExecutedPrice* order_executed = dynamic_cast<ITCH_OrderExecutedPrice*>(msg.get());
          _parse_ITCH_order_executed_price(*order_executed, buffer);
        }
        break;
      case 'X':
        {
          ITCH_OrderCancel* order_cancel = dynamic_cast<ITCH_OrderCancel*>(msg.get());
          _parse_ITCH_order_cancel(*order_cancel, buffer);
        }
        break;
      default:
        // TODO: Log error
        std::cout << "ITCH_Parser::parse_ITCH_outbound: Unknown message type: " << msg->otype << std::endl;
        break;
    }
  }

  void ITCH_Parser::_parse_ITCH_add_order(ITCH_AddOrderMPID& msg, unsigned char buffer[]) {
    memcpy(buffer, &msg.otype, 1);
    memcpy(buffer + 1, &msg.stock_locate, 2);
    memcpy(buffer + 3, &msg.tracking_number, 2);
    memcpy(buffer + 5, &msg.timestamp, 8); // TODO: Check for errors
    memcpy(buffer + 11+2, &msg.order_ref_num, 8);
    memcpy(buffer + 19+2, &msg.buy_sell_indicator, 1);
    memcpy(buffer + 20+2, &msg.shares, 4);
    memcpy(buffer + 24+2, &msg.stock, 8);
    memcpy(buffer + 32+2, &msg.price, 4);
    memcpy(buffer + 36+2, &msg.mpid, 4); // Attribution
  }

  void ITCH_Parser::_parse_ITCH_order_executed_price(ITCH_OrderExecutedPrice& msg, unsigned char buffer[]) {
    memcpy(buffer, &msg.otype, 1);
    memcpy(buffer + 1, &msg.stock_locate, 2);
    memcpy(buffer + 3, &msg.tracking_number, 2);
    memcpy(buffer + 5, &msg.timestamp, 8); // TODO: Check for errors
    memcpy(buffer + 11+2, &msg.order_ref_num, 8);
    memcpy(buffer + 19+2, &msg.executed_shares, 4);
    memcpy(buffer + 23+2, &msg.match_num, 8);
    memcpy(buffer + 31+2, &msg.printable, 1);
    memcpy(buffer + 32+2, &msg.execution_price, 4);
  }

  void ITCH_Parser::_parse_ITCH_order_cancel(ITCH_OrderCancel& msg, unsigned char buffer[]) {
    memcpy(buffer, &msg.otype, 1);
    memcpy(buffer + 1, &msg.stock_locate, 2);
    memcpy(buffer + 3, &msg.tracking_number, 2);
    memcpy(buffer + 5, &msg.timestamp, 8); // TODO: Check for errors
    memcpy(buffer + 11+2, &msg.order_ref_num, 8);
    memcpy(buffer + 19+2, &msg.canceled_shares, 4);
  }
}